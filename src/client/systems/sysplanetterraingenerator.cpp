/*
* Copyright 2021 Conquer Space
*/
#include "client/systems/sysplanetterraingenerator.h"

#include <spdlog/spdlog.h>

#include <cmath>

void conquerspace::client::systems::TerrainImageGenerator::GenerateTerrain(int octaves, int size) {
    noise::module::Perlin noise_module;
    noise_module.SetOctaveCount(octaves);
    noise_module.SetSeed(seed);
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

    renderer.SetSourceNoiseMap(noise_map);
    renderer.SetDestImage(albedo_map);

    renderer.ClearGradient();

    // TODO(EhWhoAmI): Need to add different types of terrain.
    renderer.AddGradientPoint(-1.0000, utils::Color(0, 0, 128, 255));  // deeps
    renderer.AddGradientPoint(-0.2500, utils::Color(0, 0, 255, 255));  // shallow
    renderer.AddGradientPoint(0.0000, utils::Color(0, 128, 255, 255));  // shore
    renderer.AddGradientPoint(0.0625, utils::Color(240, 240, 64, 255));  // sand
    renderer.AddGradientPoint(0.1250, utils::Color(32, 160, 0, 255));  // grass
    renderer.AddGradientPoint(0.3750, utils::Color(224, 224, 0, 255));  // dirt
    renderer.AddGradientPoint(0.7500, utils::Color(128, 128, 128, 255));  // rock
    renderer.AddGradientPoint(1.0000, utils::Color(255, 255, 255, 255));  // snow
    renderer.EnableLight();
    renderer.SetLightContrast(3.0);
    renderer.SetLightBrightness(2.0);
    renderer.SetLightAzimuth(0);

    renderer.Render();
}

void conquerspace::client::systems::TerrainImageGenerator::ClearData() {
    height_map.ReclaimMem();
    albedo_map.ReclaimMem();
}
