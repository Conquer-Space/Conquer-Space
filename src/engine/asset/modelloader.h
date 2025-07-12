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
#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <map>
#include <string>
#include <vector>

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

#include "engine/asset/assetmanager.h"
#include "engine/asset/assetprototype.h"
#include "engine/graphics/mesh.h"
#include "engine/graphics/model.h"
#include "engine/graphics/texture.h"

namespace cqsp::asset {
class IOSystem;
class IOStream : public Assimp::IOStream {
    friend class IOSystem;

 protected:
    explicit IOStream(IVirtualFilePtr _ivfp) : ivfp(_ivfp) {}

    IVirtualFilePtr ivfp;

 public:
    size_t Read(void* pvBuffer, size_t pSize, size_t pCount);
    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) {
        return pSize;  // We won't ever write because we are only going to open game files
    }
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin);
    size_t Tell() const;
    size_t FileSize() const;
    void Flush();

    void Close();
};

class IOSystem : public Assimp::IOSystem {
    VirtualMounter* mount;

 public:
    explicit IOSystem(VirtualMounter* mount);
    ~IOSystem() = default;

    bool Exists(const char* file) const override;
    char getOsSeparator() const override { return '/'; }

    Assimp::IOStream* Open(const char* pFile, const char* pMode) override;
    void Close(Assimp::IOStream* pFile) override;
};

struct MeshPrototype {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::string> textures;
    int material_id;
};

struct ModelTexturePrototype {
    int width;
    int height;
    int channels;

    unsigned char* texture_data;
};

struct MaterialPrototype {
    std::map<std::string, std::string> texture_map;
    std::map<std::string, glm::vec3> base_map;
};

struct ModelPrototype : public AssetPrototype {
 public:
    std::vector<MeshPrototype> prototypes;
    std::map<std::string, ModelTexturePrototype> texture_map;
    std::map<int, MaterialPrototype> material_map;
    std::map<std::string, int> texture_idx_map;  // Probably should be per material but
                                                 // Let's figure that out later
    std::string shader;

    int GetPrototypeType() { return PrototypeType::MODEL; }
};

struct ModelLoader {
    int m_count = 0;
    ModelPrototype* model_prototype;
    const aiScene* scene;
    std::string asset_path;
    ModelLoader(const aiScene* _scene, std::string asset_path) : scene(_scene), asset_path(asset_path) {
        model_prototype = new ModelPrototype();
    }

    void LoadModel();

    void LoadNode(aiNode* node);

    void LoadMesh(aiMesh* mesh);

    void LoadMaterials();

    void LoadMaterial(int idx, aiMaterial* material);

    bool LoadMaterialTexture(aiMaterial* material, const aiTextureType type, std::string& str);
};

void LoadModelPrototype(ModelPrototype* prototype, Model* asset);

void LoadModelData(engine::Mesh* mesh, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
}  // namespace cqsp::asset
