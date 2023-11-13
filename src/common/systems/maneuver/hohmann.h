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

#include <optional>
#include <utility>

#include <glm/vec3.hpp>

#include "common/components/orbit.h"

namespace cqsp::common::systems {
typedef std::pair<glm::dvec3, double> Maneuver_t;
typedef std::pair<Maneuver_t, Maneuver_t> HohmannPair_t;
std::optional<HohmannPair_t> HohmannTransfer(const components::types::Orbit& orbit, double altitude);
}  // namespace cqsp::common::systems
