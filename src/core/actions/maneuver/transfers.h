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

#include "core/components/coordinates.h"
#include "core/components/maneuver.h"
#include "core/components/orbit.h"
#include "core/universe.h"

namespace cqsp::core::systems {
components::Maneuver_t TransferFromBody(Universe& universe, const components::types::Orbit& orbit,
                                        const components::types::Kinematics& kinematics, double altitude);
glm::dvec3 GetBodyVelocityVectorInOrbitPlane(const components::types::Orbit& orbit,
                                             const components::types::Kinematics& body_kinematics);
double GetBodyVelocityVectorInOrbitPlaneTrueAnomaly(const components::types::Orbit& orbit,
                                                    const components::types::Kinematics& kinematics,
                                                    const components::types::Kinematics& orbiting_kinematics);
}  // namespace cqsp::core::systems
