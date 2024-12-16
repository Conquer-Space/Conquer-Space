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
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/asset/asset.h"
#include "engine/graphics/mesh.h"
#include "engine/graphics/texture.h"

namespace cqsp::asset {

struct Material {
    std::vector<asset::Texture*> diffuse;
    std::vector<asset::Texture*> specular;
    std::vector<asset::Texture*> ambient;
    std::vector<asset::Texture*> height;
    std::vector<asset::Texture*> metallic;
    std::vector<asset::Texture*> roughness;
    glm::vec3 base_diffuse;
    glm::vec3 base_specular;
    glm::vec3 base_ambient;
    glm::vec3 base_emissive;
    glm::vec3 base_transparent;

    Material() = default;
    ~Material() {
        for (auto& texture : diffuse) {
            delete texture;
        }
        for (auto& texture : specular) {
            delete texture;
        }
        for (auto& texture : ambient) {
            delete texture;
        }
        for (auto& texture : height) {
            delete texture;
        }
        for (auto& texture : metallic) {
            delete texture;
        }
        for (auto& texture : roughness) {
            delete texture;
        }
    }
};

struct ModelMesh : public engine::Mesh {
    int material;
};

typedef std::shared_ptr<ModelMesh> ModelMesh_t;

struct Model : public Asset {
    std::vector<ModelMesh_t> meshes;
    std::map<int, Material> materials;
    glm::vec3 scale;

    AssetType GetAssetType() override { return AssetType::MODEL; }

    void Draw(ShaderProgram* shader) {
        for (auto& model_mesh : meshes) {
            // Set the texture of the model mesh
            // Set the material
            // ISS just has a base diffuse color
            auto& material = materials[model_mesh->material];

            shader->setVec3("diffuse", material.base_diffuse);
            model_mesh->Draw();
        }
    }
};

struct Vertex {
 public:
    static const int MAX_BONE_INFLUENCE = 4;
    // position
    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    // normal
    glm::vec3 normal = glm::vec3(0.f, 0.f, 0.f);
    // texCoords
    glm::vec2 texCoords = glm::vec2(0.f, 0.f);
    // tangent
    glm::vec3 tangent = glm::vec3(0.f, 0.f, 0.f);
    // bitangent
    glm::vec3 bitangent = glm::vec3(0.f, 0.f, 0.f);
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};
}  // namespace cqsp::asset
