/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <noise/noise.h>

#include <noiseutils.h>

namespace conquerspace {
namespace client {
namespace systems {
class TerrainImageGenerator {
 public:
    void GenerateTerrain(int, int);
    void ClearData();

    noise::utils::Image& GetHeightMap() { return height_map; }
    noise::utils::Image& GetAlbedoMap() { return albedo_map; }
 private:
    int seed;
    noise::utils::Image height_map;
    noise::utils::Image albedo_map;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
