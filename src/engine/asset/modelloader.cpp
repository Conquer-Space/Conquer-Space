/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "engine/graphics/mesh.h"
#include "engine/graphics/texture.h"

#define SET_MATERIAL_TEXTURES(part)                      \
    for (auto&(part) : material_prototype.second.part) { \
        material.part.push_back(texture_map[part]);      \
    }

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

void GenerateMesh(engine::Mesh& mesh, std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
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
    mesh.indicies = indices.size();
}

void LoadModelPrototype(ModelPrototype* prototype, Model* asset) {
    for (auto& mesh_type : prototype->prototypes) {
        ModelMesh_t mesh = std::make_shared<ModelMesh>();
        asset::LoadModelData(mesh.get(), mesh_type.vertices, mesh_type.indices);
        mesh->material = mesh_type.material_id;
        asset->meshes.push_back(mesh);
    }
    // Load textures
    std::map<std::string, asset::Texture*> texture_map;
    for (auto& textures : prototype->texture_map) {
        // Set the map
        asset::Texture* texture = new asset::Texture();
        auto& tex_prototype = textures.second;
        asset::CreateTexture(*texture, tex_prototype.texture_data, tex_prototype.width, tex_prototype.height,
                             tex_prototype.channels);
        // Insert the textures into the model
        stbi_image_free(tex_prototype.texture_data);
        texture_map[textures.first] = texture;
    }

    // Load materials
    for (auto& material_prototype : prototype->material_map) {
        // Loop through the list
        Material& material = asset->materials[material_prototype.first];
        // Set all the values
        material.attributes.reserve(material_prototype.second.base_map.size());
        for (auto& pair : material_prototype.second.base_map) {
            // Copy the base elements
            material.attributes.push_back(std::make_pair(pair.first, pair.second));
        }
        for (auto& [key, texture] : material_prototype.second.texture_map) {
            // Look for the names and assign the integer value
            // Check if it exists in the
            if (prototype->texture_idx_map.contains(key)) {
                material.textures.push_back(std::make_pair(prototype->texture_idx_map[key], texture_map[texture]));
            }
        }
    }
    // Set the shader
}

void LoadModelData(engine::Mesh* mesh, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    GenerateMesh(*mesh, std::move(vertices), std::move(indices));
}

bool ModelLoader::LoadMaterialTexture(aiMaterial* material, const aiTextureType type, std::string& str) {
    if (material->GetTextureCount(type) == 0) {
        return false;
    }
    aiString path;
    material->GetTexture(type, 0, &path);
    std::string path_str(path.C_Str());
    if (model_prototype->texture_map.contains(path_str)) {
        // Set the path str
        str = path_str;
        return true;
    }
    ModelTexturePrototype mesh_proto;
    // Look for the relative path to the model
    // TODO(EhWhoAmI): Load it from our vfs
    auto tex_path = std::filesystem::path(asset_path) / path_str;
    stbi_set_flip_vertically_on_load((int)true);
    mesh_proto.texture_data =
        stbi_load(tex_path.string().c_str(), &mesh_proto.width, &mesh_proto.height, &mesh_proto.channels, 0);
    if (mesh_proto.texture_data == NULL) {
        ENGINE_LOG_WARN("Error loading texture {} ()", path_str, asset_path);
        return false;
    }
    model_prototype->texture_map[path_str] = mesh_proto;
    // return the name of the file
    str = path_str;
    return true;
}

void ModelLoader::LoadModel() {
    LoadNode(scene->mRootNode);
    LoadMaterials();
}

void ModelLoader::LoadNode(aiNode* node) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        LoadMesh(mesh);
    }

    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        LoadNode(node->mChildren[i]);
    }
}

void ModelLoader::LoadMesh(aiMesh* mesh) {
    MeshPrototype mesh_prototype;
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
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
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
        mesh_prototype.vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            mesh_prototype.indices.push_back(face.mIndices[j]);
        }
    }
    if (mesh->mMaterialIndex >= 0) {
        // Set the material index
        mesh_prototype.material_id = mesh->mMaterialIndex;
    }
    model_prototype->prototypes.push_back(mesh_prototype);
}

void ModelLoader::LoadMaterials() {
    ENGINE_LOG_INFO("Loading {} materials", scene->mNumMaterials);
    for (int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        // Alright in theory you can merge multiple textures to the same material
        // But that will take way too much effort
        // So I will go ahead and avoid that
        LoadMaterial(i, mat);
    }
}

void ModelLoader::LoadMaterial(int idx, aiMaterial* material) {
    // Since our rendering model is to "apply" a shader to a mesh,
    MaterialPrototype prototype;
    aiColor3D color(0.f, 0.f, 0.f);
    ENGINE_LOG_INFO("Loading material {}", material->GetName().C_Str());
    // Get the color and then apply it or something
    // Assimp will always give us a value for each of these parameters
    // However there really isn't a good way to identify when we should use
    // So we will probably need to cancel them based off which texture attribute
    // we are able to retrieve
    std::string buf;
    if (!LoadMaterialTexture(material, aiTextureType_SPECULAR, buf)) {
        // Load the 3d vector
        material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        prototype.base_map["specular"] = glm::vec3(color.r, color.g, color.b);
    } else {
        prototype.texture_map["specular"] = buf;
    }

    if (!LoadMaterialTexture(material, aiTextureType_DIFFUSE, buf)) {
        // Load the 3d vector
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        prototype.base_map["diffuse"] = glm::vec3(color.r, color.g, color.b);
    } else {
        prototype.texture_map["diffuse"] = buf;
    }

    if (!LoadMaterialTexture(material, aiTextureType_AMBIENT, buf)) {
        // Load the 3d vector
        material->Get(AI_MATKEY_COLOR_AMBIENT, color);
        prototype.base_map["ambient"] = glm::vec3(color.r, color.g, color.b);
    } else {
        prototype.texture_map["ambient"] = buf;
    }

    if (!LoadMaterialTexture(material, aiTextureType_EMISSIVE, buf)) {
        // Load the 3d vector
        material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
        prototype.base_map["emissive"] = glm::vec3(color.r, color.g, color.b);
    } else {
        prototype.texture_map["emissive"] = buf;
    }

    if (LoadMaterialTexture(material, aiTextureType_HEIGHT, buf)) {
        // Load the 3d vector
        prototype.texture_map["height"] = buf;
    }

    if (LoadMaterialTexture(material, aiTextureType_METALNESS, buf)) {
        // Load the 3d vector
        prototype.texture_map["metalness"] = buf;
    }

    if (LoadMaterialTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, buf)) {
        // Load the 3d vector
        prototype.texture_map["roughness"] = buf;
    }

    material->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
    prototype.base_map["transparent"] = glm::vec3(color.r, color.g, color.b);

    ENGINE_LOG_INFO("Loading {} properties", material->mNumProperties);

    ENGINE_LOG_INFO("Shading model: {}", material->GetTextureCount(aiTextureType_METALNESS));

    // Check if it's pbr and if it's pbr

    model_prototype->material_map[idx] = prototype;
}
}  // namespace cqsp::asset
