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

#include <deque>
#include <utility>

#include "common/components/coordinates.h"

namespace cqsp::common::components {
// Datatype returned by a command
typedef std::pair<glm::dvec3, double> Maneuver_t;
typedef std::pair<Maneuver_t, Maneuver_t> HohmannPair_t;

/// <summary>
/// </summary>
struct Maneuver {
    /// <summary>
    /// Delta v applied with respect to the orbital vector
    /// positive y is prograde, negative y is retrograde
    /// I'm not sure what the rest are, but I think
    /// positve x is radial out, negative x is radial in
    /// positve z is normal, negative z is anti-normal
    /// </summary>
    glm::dvec3 delta_v;
    double time;
    Maneuver() = default;
    explicit Maneuver(const std::pair<glm::dvec3, double>& maneuver) : delta_v(maneuver.first), time(maneuver.second) {}
    explicit Maneuver(const std::pair<glm::dvec3, double>& maneuver, double current_time)
        : delta_v(maneuver.first), time(maneuver.second + current_time) {}
};

struct CommandQueue {
    std::deque<Maneuver> maneuvers;
    std::deque<entt::entity> commands;
    CommandQueue() = default;
    auto begin() { return maneuvers.begin(); }
    auto end() { return maneuvers.end(); }
};
}  // namespace cqsp::common::components
