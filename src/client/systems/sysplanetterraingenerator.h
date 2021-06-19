/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <noise/noise.h>

#include "noiseutils.h"

namespace conquerspace {
namespace client {
namespace systems {
class TerrainImageGenerator {
 public:
    void GenerateTerrain(int, int);

    noise::utils::Image& GetRoughnessMap() { return roughness_map; }
    noise::utils::Image& GetAlbedoMap() { return albedo_map; }
 private:
    int seed;
    noise::utils::Image roughness_map;
    noise::utils::Image albedo_map;
};
}
}  // namespace client
}  // namespace conquerspace