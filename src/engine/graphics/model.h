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
#include "engine/graphics/shader.h"
#include "engine/graphics/texture.h"

namespace cqsp::asset {
struct Material {
    // Assign each texture to an id
    std::vector<std::pair<int, asset::Texture*>> textures;
    std::vector<std::pair<std::string, glm::vec3>> attributes;
    Material() = default;
    ~Material() {
        for (auto& pair : textures) {
            delete pair.second;
        }
    }

    void SetShader(ShaderProgram_t& shader) {
        for (auto& pair : textures) {
            shader->bindTexture(pair.first, pair.second->id);
        }
        for (auto& pair : attributes) {
            shader->setVec3(pair.first, pair.second);
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
    // In theory each material could have a different shader,
    // but for now we will generalize for the entire model
    ShaderProgram_t shader;
    std::string shader_name;

    void PostLoad(AssetManager&);

    AssetType GetAssetType() override { return AssetType::MODEL; }

    void Draw(ShaderProgram_t shader) {
        for (auto& model_mesh : meshes) {
            // Set the texture of the model mesh
            // Set the material
            // ISS just has a base diffuse color
            auto& material = materials[model_mesh->material];
            material.SetShader(shader);
            model_mesh->Draw();
        }
    }

    void Draw() {
        for (auto& model_mesh : meshes) {
            // Set the texture of the model mesh
            // Set the material
            // ISS just has a base diffuse color
            auto& material = materials[model_mesh->material];
            material.SetShader(shader);
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
