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
#include "common/systems/maneuver/rendezvous.h"

namespace cqsp::common::systems {
using namespace components::types;  // NOLINT
std::pair<glm::dvec3, double> CoplanarIntercept(const components::types::Orbit& start_orbit,
                                                const components::types::Orbit& end_orbit, double epoch) {
    // They need to be the same plane, but let's ignore that
    // Also needs to be circular
    // Get true anomaly
    double a = GetTrueAnomaly(start_orbit, epoch);
    double b = GetTrueAnomaly(end_orbit, epoch);
    // Get relative theta
    b = b + end_orbit.w;
    // Get the theta, and make sure that we can align up the orbit
    return std::pair<glm::dvec3, double>();
}
}  // namespace cqsp::common::systems
