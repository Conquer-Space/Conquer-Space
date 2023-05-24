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
#include "client/systems/sysplanetterraingenerator.h"

#include <spdlog/spdlog.h>

#include <cmath>

#include <tracy/Tracy.hpp>

#include "common/components/bodies.h"

using cqsp::client::systems::TerrainImageGenerator;

void TerrainImageGenerator::GenerateTerrain(cqsp::common::Universe& universe, int octaves, int size) {
    ZoneScoped;
    noise::module::Perlin noise_module;
    noise_module.SetOctaveCount(octaves);
    noise_module.SetNoiseQuality(noise::QUALITY_FAST);
    noise_module.SetSeed(terrain.seed);
    noise_module.SetFrequency(2);

    noise::utils::NoiseMap noise_map;
    int textureWidth = std::pow(2, size);
    int textureHeight = std::pow(2, size);
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(noise_module);
    heightMapBuilder.SetDestNoiseMap(noise_map);
    heightMapBuilder.SetDestSize(textureWidth, textureHeight);

    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);

    heightMapBuilder.Build();

    noise::utils::RendererImage renderer;
    renderer.SetSourceNoiseMap(noise_map);
    renderer.SetDestImage(height_map);
    renderer.Render();

    renderer.SetDestImage(albedo_map);

    renderer.ClearGradient();

    auto& terrain_data = universe.get<cqsp::common::components::bodies::TerrainData>(terrain.terrain_type);
    for (auto it = terrain_data.data.begin(); it != terrain_data.data.end(); it++) {
        renderer.AddGradientPoint(it->first, noise::utils::Color(std::get<0>(it->second), std::get<1>(it->second),
                                                                 std::get<2>(it->second), std::get<3>(it->second)));
    }
    renderer.Render();
}

void TerrainImageGenerator::GenerateHeightMap(int octaves, int size) {
    noise::module::Perlin noise_module;
    noise_module.SetOctaveCount(octaves);
    noise_module.SetNoiseQuality(noise::QUALITY_FAST);
    noise_module.SetSeed(terrain.seed);
    noise_module.SetFrequency(2);

    noise::utils::NoiseMap noise_map;
    int textureWidth = std::pow(2, size);
    int textureHeight = std::pow(2, size);
    utils::NoiseMapBuilderSphere heightMapBuilder;
    heightMapBuilder.SetSourceModule(noise_module);
    heightMapBuilder.SetDestNoiseMap(noise_map);
    heightMapBuilder.SetDestSize(textureWidth, textureHeight);

    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);

    heightMapBuilder.Build();

    noise::utils::RendererImage renderer;
    renderer.SetSourceNoiseMap(noise_map);
    renderer.SetDestImage(height_map);
    renderer.Render();
}

void TerrainImageGenerator::ClearData() {
    height_map.ReclaimMem();
    albedo_map.ReclaimMem();
}
