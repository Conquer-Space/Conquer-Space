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

#include <entt/entity/entity.hpp>

namespace cqsp::core::components {
// https://forum.kerbalspaceprogram.com/index.php?/topic/84752-how-do-i-calculate-payload-mass-to-orbit/
// TODO(EhWhoAmI): In the future we should also support reusable planes and other types of launch vehicles as well
struct LaunchVehicle {
    double delta_v;
    double reliability;
    double fairing_size;  // Maybe specific dimensions in the future
    double mass_to_orbit;
};

struct SpaceCapability {
    std::vector<entt::entity> launch_vehicle_list;
};
}  // namespace cqsp::core::components
