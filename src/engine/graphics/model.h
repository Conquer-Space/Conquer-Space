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

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/asset/asset.h"
#include "engine/graphics/mesh.h"
#include "engine/graphics/texture.h"

namespace cqsp::asset {
struct Model : public Asset {
    std::vector<engine::Mesh> meshes;
    std::vector<cqsp::asset::Texture*> textures;

    AssetType GetAssetType() override { return AssetType::MODEL; }
};

struct Vertex {
 public:
    static const int MAX_BONE_INFLUENCE = 4;
    // position
    glm::vec3 position;
    // normal
    glm::vec3 normal;
    // texCoords
    glm::vec2 texCoords;
    // tangent
    glm::vec3 tangent;
    // bitangent
    glm::vec3 bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};
}  // namespace cqsp::asset
