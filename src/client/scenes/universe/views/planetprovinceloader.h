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

#include "core/universe.h"
#include "engine/application.h"
#include "stb_image.h"  // NOLINT: The linter is rather annoying with stb

namespace cqsp::client::systems {
class PlanetProvinceLoader {
 public:
    PlanetProvinceLoader(engine::Application &app, core::Universe &universe, entt::entity body)
        : app(app), universe(universe), body(body) {}
    ~PlanetProvinceLoader();
    void LoadProvinces();

 private:
    bool LoadPlanetTexture();
    bool CheckConditions();
    void BuildIndexMap();
    void BuildPlanetTexture();
    void GeneratePlanetProvinceMap();

    entt::entity body;
    core::Universe &universe;
    engine::Application &app;

    // stbi information
    int province_width;
    int province_height;
    int comp;

    // Counter to assign to the array of colors
    uint16_t current_province_idx = 1;
    stbi_uc *image = nullptr;
};
}  // namespace cqsp::client::systems
