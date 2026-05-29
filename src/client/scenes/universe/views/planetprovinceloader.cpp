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
#include "client/scenes/universe/views/planetprovinceloader.h"

#include <chrono>

#include "client/components/planetrendering.h"
#include "core/components/surface.h"
#include "engine/asset/textasset.h"
#include "engine/graphics/texture.h"
#include "stb_image.h"  // NOLINT: The linter is rather annoying with stb

namespace cqsp::client::systems {
namespace components = core::components;
using asset::Texture;
using cqsp::client::components::PlanetTexture;

bool PlanetProvinceLoader::LoadPlanetTexture() {
    auto& province_map = universe.get<components::ProvincedPlanet>(body);
    asset::BinaryAsset* bin_asset = app.GetAssetManager().GetAsset<asset::BinaryAsset>(province_map.province_map);
    if (bin_asset == nullptr) {
        SPDLOG_ERROR("Could not find the planet province map {}", province_map.province_map);
        return false;
    }

    uint64_t file_size = bin_asset->data.size();
    // Don't flip vertically
    stbi_set_flip_vertically_on_load(0);
    image = stbi_load_from_memory(bin_asset->data.data(), file_size, &province_width, &province_height, &comp, 0);
    return image != nullptr;
}

bool PlanetProvinceLoader::CheckConditions() {
    if (!universe.any_of<components::ProvincedPlanet>(body)) {
        return false;
    }
}

PlanetProvinceLoader::~PlanetProvinceLoader() {
    if (image != nullptr) {
        stbi_image_free(image);
    }
}

void PlanetProvinceLoader::LoadProvinces() {
    auto start = std::chrono::high_resolution_clock::now();
    if (!CheckConditions()) {
        return;
    }
    if (!LoadPlanetTexture()) {
        return;
    }

    BuildPlanetTexture();
    BuildIndexMap();
    GeneratePlanetProvinceMap();

    auto end = std::chrono::high_resolution_clock::now();
    int len = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    SPDLOG_INFO("Took {} ms to load province image", len);
}

void PlanetProvinceLoader::BuildIndexMap() {
    auto& planet_texture = universe.get_or_emplace<PlanetTexture>(body);

    // If the province doesn't have a pixel then we will not add the index, so the index ends up being 0.
    // We should probably fix this by not having any provinces with no pixels
    // As a temp fix, let's sort through all the provinces and figure out what province exists or not and
    // then add it into the map so that it doesn't just go straight to zero
    auto& settlements = universe.get<components::Settlements>(body);
    for (entt::entity province : settlements.provinces) {
        if (!planet_texture.province_index_map.contains(province)) {
            planet_texture.province_index_map[province] = current_province_idx;
            current_province_idx++;
        }
    }
}

void PlanetProvinceLoader::BuildPlanetTexture() {
    auto& planet_texture = universe.get_or_emplace<PlanetTexture>(body);
    planet_texture.province_map.reserve(static_cast<size_t>(province_height) * static_cast<size_t>(province_width));
    planet_texture.province_indices.reserve(static_cast<size_t>(province_height) * static_cast<size_t>(province_width));
    // Counter to assign to the array of colors
    planet_texture.province_index_map[entt::null] = 0;
    // We expect the province map will be the same dimensions as the province texture, so it should be fine?
    for (int idx = 0; idx < province_width * province_height; idx++) {
        // Position on the map
        int pos = idx * comp;
        int i = components::ProvinceColor::toInt(image[pos], image[pos + 1], image[pos + 2]);
        int x = idx % province_width;
        int y = idx / province_width;
        if (universe.province_colors[body].find(i) != universe.province_colors[body].end()) {
            entt::entity province_id = universe.province_colors[body][i];
            planet_texture.province_map.push_back(province_id);
            if (!planet_texture.province_index_map.contains(province_id)) {
                planet_texture.province_index_map[province_id] = current_province_idx;
                current_province_idx++;
            }
            planet_texture.province_indices.push_back(planet_texture.province_index_map[province_id]);

        } else {
            // Most likely ocean
            // Maybe next time we should have ocean provinces
            planet_texture.province_map.push_back(entt::null);
            planet_texture.province_indices.push_back(0);
        }
    }

    assert(planet_texture.province_indices.size() ==
           static_cast<size_t>(province_width) * static_cast<size_t>(province_height));
    // Check that our province map and indices are the right size.
    assert(planet_texture.province_map.size() == planet_texture.province_indices.size());

    planet_texture.has_provinces = true;
    planet_texture.width = province_width;
    planet_texture.height = province_height;
}

/**
 * Generates the OpenGL textures for the province map and the colors we are to assign to the province map
 */
void PlanetProvinceLoader::GeneratePlanetProvinceMap() {
    assert(universe.all_of<PlanetTexture>(body));
    auto& data = universe.get<PlanetTexture>(body);

    // Generate int texture for assigning color indices
    unsigned int texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, province_width, province_height, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
                 data.province_indices.data());
    data.province_index_texture = new Texture();
    data.province_index_texture->id = texid;
    data.province_index_texture->width = province_width;
    data.province_index_texture->height = province_height;
    data.province_index_texture->texture_type = GL_TEXTURE_2D;

    // Now let's generate our indices for the color for the province
    const size_t color_count = static_cast<size_t>(current_province_idx);
    data.province_colors.reserve(color_count);
    // Now let's just assign random colors...
    for (size_t i = 0; i < color_count; i++) {
        data.province_colors.emplace_back(0.f);
    }

    // Generate TBO
    GLuint tbo_buffer;
    glGenBuffers(1, &tbo_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_buffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(decltype(data.province_colors)::value_type) * data.province_colors.size(),
                 static_cast<const void*>(data.province_colors.data()), GL_STATIC_DRAW);

    GLuint tbo_texture;
    glGenTextures(1, &tbo_texture);
    glBindTexture(GL_TEXTURE_BUFFER, tbo_texture);

    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo_buffer);
    // Now let's save this texture
    data.province_color_map = new asset::TBOTexture();
    data.province_color_map->id = tbo_texture;
    dynamic_cast<asset::TBOTexture*>(data.province_color_map)->buffer_id = tbo_buffer;
    data.province_color_map->texture_type = GL_TEXTURE_BUFFER;
}
}  // namespace cqsp::client::systems
