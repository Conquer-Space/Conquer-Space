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

#include <vector>

#include <entt/entt.hpp>

#include "core/components/units.h"

namespace cqsp::core::components::ships {
struct Ship {};

struct Crash {};

struct Fleet {
    unsigned int echelon = 0;
    std::vector<entt::entity> subfleets;
    std::vector<entt::entity> ships;
    entt::entity parent_fleet = entt::null;
    entt::entity owner;
    Fleet(entt::entity parent_fleet, entt::entity _owner, unsigned int _echelon)
        : parent_fleet(parent_fleet), owner(_owner), echelon(_echelon) {}
    // creates top level fleet
    explicit Fleet(entt::entity _owner) : Fleet(entt::null, _owner, 0) {}
};
}  // namespace cqsp::core::components::ships
