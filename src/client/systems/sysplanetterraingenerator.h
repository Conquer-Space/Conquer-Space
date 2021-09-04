/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <noise/noise.h>

#include <noiseutils.h>

namespace cqsp {
namespace client {
namespace systems {
class TerrainImageGenerator {
 public:
    void GenerateTerrain(int octaves, int size);
    void GenerateHeightMap(int octaves, int size);
    void ClearData();

    noise::utils::Image& GetHeightMap() { return height_map; }
    noise::utils::Image& GetAlbedoMap() { return albedo_map; }

    int seed;
 private:
    noise::utils::Image height_map;
    noise::utils::Image albedo_map;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
