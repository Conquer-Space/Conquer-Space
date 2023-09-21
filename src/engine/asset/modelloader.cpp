/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "engine/asset/modelloader.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <cstddef>
#include <vector>

#include "engine/graphics/mesh.h"
#include "engine/graphics/texture.h"

namespace cqsp::asset {
IOSystem::IOSystem(VirtualMounter* mount) : mount(mount) {}
bool IOSystem::Exists(const char* file) const { return mount->Exists(file); }

void IOSystem::Close(Assimp::IOStream* pFile) { dynamic_cast<IOStream*>(pFile)->Close(); }

// @param pMode Desired file I/O mode. Required are: "wb", "w", "wt",
// *"rb", "r", "rt".*
Assimp::IOStream* IOSystem::Open(const char* pFile, const char* pMode) {
    // Need to implement write to mount
    if (strlen(pMode) == 0) {
        return nullptr;
    }
    if (pMode[0] == 'r') {
        // Check if it's binary
        if (strlen(pMode) == 2 && pMode[1] == 'b') {
            // Open binary
            return new IOStream(mount->Open(pFile, FileModes::Binary));
        } else if (strlen(pMode) == 1 || (strlen(pMode) == 2 && pMode[1] == 't')) {
            // Open text
            return new IOStream(mount->Open(pFile, FileModes::Text));
        }
    }
    return nullptr;
}

void IOStream::Close() { ivfp.reset(); }

size_t IOStream::Read(void* pvBuffer, size_t pSize, size_t pCount) {
    size_t alloc = pSize * pCount / sizeof(uint8_t);
    if (alloc > ivfp->Size()) {
        alloc = ivfp->Size();
    }
    ivfp->Read(static_cast<uint8_t*>(pvBuffer), alloc);
    return alloc / pSize;
}

aiReturn IOStream::Seek(size_t pOffset, aiOrigin pOrigin) {
    /** Beginning of the file */
    Offset offset;
    switch (pOrigin) {
        default:
        case aiOrigin_SET:
            offset = Offset::Beg;
            break;
        case aiOrigin_CUR:
            offset = Offset::Cur;
            break;
        case aiOrigin_END:
            offset = Offset::End;
            break;
    }
    return static_cast<int>(ivfp->Seek((long)pOffset, offset)) == 0 ? aiReturn::aiReturn_SUCCESS
                                                                    : aiReturn::aiReturn_FAILURE;
}
size_t IOStream::Tell() const { return ivfp->Tell(); }
size_t IOStream::FileSize() const { return ivfp->Size(); }
void IOStream::Flush() {}

void ModelLoader::LoadNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        LoadMesh(mesh, scene);
    }
    LoadMaterials(scene);
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        LoadNode(node->mChildren[i], scene);
    }
}
void ModelLoader::LoadMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }
        // does the mesh contain texture coordinates?
        if (mesh->mTextureCoords[0] != nullptr) {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.x = mesh->mTextureCoords[0][i].y;
            // tangent
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
            // bitangent
            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    // Generate mesh
    GenerateMesh(vertices, indices);
    if (mesh->mMaterialIndex >= 0) {
        LoadMaterial(scene->mMaterials[mesh->mMaterialIndex]);
    }
}

// https://assimp.sourceforge.net/lib_html/material_8h.html#a7dd415ff703a2cc53d1c22ddbbd7dde0
// https://assimp.sourceforge.net/lib_html/structai_material.html
void ModelLoader::LoadMaterials(const aiScene* scene) {
    for (int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        LoadMaterialTexture(mat, aiTextureType_SPECULAR);
        LoadMaterialTexture(mat, aiTextureType_DIFFUSE);
        LoadMaterialTexture(mat, aiTextureType_HEIGHT);
        LoadMaterialTexture(mat, aiTextureType_AMBIENT);
        // Save the material?
    }
}

void ModelLoader::LoadMaterial(aiMaterial* material) {
    LoadMaterialTexture(material, aiTextureType_SPECULAR);
    LoadMaterialTexture(material, aiTextureType_DIFFUSE);
    LoadMaterialTexture(material, aiTextureType_HEIGHT);
    LoadMaterialTexture(material, aiTextureType_AMBIENT);
}

void ModelLoader::LoadMaterialTexture(aiMaterial* material, const aiTextureType& type) {
    for (int i = 0; i < material->GetTextureCount(type); i++) {
        aiString path;
        material->GetTexture(type, i, &path);
        Texture texure;
        // Load file from memory
        int width;
        int height;
        int channels;
        unsigned char* c = stbi_load(path.C_Str(), &width, &height, &channels, 0);

        // Check if the thing has already been loaded, and then
        CreateTexture(texure, c, width, height, channels);
        // Then add it to the map or something
    }
}

void ModelLoader::GenerateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
    engine::Mesh& mesh = model->mesh;
    mesh.buffer_type = engine::DrawType::ELEMENTS;
    GLuint VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SPDLOG_ERROR("{:x}", error);
    }

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates
    // perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);  // NOLINT
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));  // NOLINT
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));  // NOLINT
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));  // NOLINT
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));  // NOLINT
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));  // NOLINT

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));  // NOLINT
    glBindVertexArray(0);

    mesh.EBO = EBO;
    mesh.VAO = VAO;
    mesh.VBO = VBO;
}
}  // namespace cqsp::asset
