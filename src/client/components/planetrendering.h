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

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "engine/graphics/mesh.h"
#include "engine/graphics/texture.h"

namespace cqsp::client::components {
struct PlanetTerrainRender {
    entt::entity resource;
};

struct Offset {
    glm::vec3 offset;
};

struct TerrainTextureData {
    asset::Texture* terrain_albedo = nullptr;
    asset::Texture* heightmap = nullptr;

    void DeleteData() {
        delete terrain_albedo;
        delete heightmap;
    }
};

struct PlanetTexture {
    asset::Texture* terrain = nullptr;
    asset::Texture* normal = nullptr;
    asset::Texture* roughness = nullptr;
    asset::Texture* province_texture = nullptr;
    std::vector<entt::entity> province_map;
};

struct OrbitMesh {
    engine::Mesh_t orbit_mesh;

    ~OrbitMesh() = default;
};
}  // namespace cqsp::client::components
