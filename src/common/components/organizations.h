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

#include <entt/entt.hpp>

namespace cqsp {
namespace common {
namespace components {
/// <summary>
/// The civilization or organization that owns or governs the city
/// </summary>
struct Governed {
    entt::entity governor;
};

struct Organization {};

struct Country {
    entt::entity capital_city = entt::null;
};

struct CountryCityList {
    std::vector<entt::entity> city_list;
    std::vector<entt::entity> province_list;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
