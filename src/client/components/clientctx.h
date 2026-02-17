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

#include <array>
#include <string>
#include <variant>

#include <entt/entity/entity.hpp>

namespace cqsp::client::ctx {

static const std::array tick_speeds {1000, 500, 333, 100, 50, 10, 1, -1, -2, -5, -10};

struct StarSystemViewDebug {
    bool to_show = false;
};

struct PauseOptions {
    bool to_tick = false;
    int tick_speed = 3;
    bool tick_once = false;
};

struct HoveringItem : public std::variant<std::monostate, entt::entity, std::string> {
    using variant::variant;
};

struct SelectedCountry {};

struct SelectedProvince {};

struct GameLoad {
    std::string load_dir;
};

struct VisibleOrbit {};

enum class MapMode { NoMapMode, CountryMapMode, ProvinceMapMode };
}  // namespace cqsp::client::ctx
