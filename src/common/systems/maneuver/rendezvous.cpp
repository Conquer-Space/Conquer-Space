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

#include "common/systems/maneuver/hohmann.h"

namespace cqsp::common::systems {
using namespace components::types;  // NOLINT
std::pair<glm::dvec3, double> CoplanarIntercept(const components::types::Orbit& start_orbit,
                                                const components::types::Orbit& end_orbit, double epoch) {
    // They need to be the same plane, but let's ignore that
    // Also needs to be circular
    // TODO(EhWhoAmI): Don't calculate the vector position and measure the relative angle
    // Need to have a more geometric calculation for this
    // Get true anomaly
    double a = GetTrueAnomaly(start_orbit, epoch) + start_orbit.w;
    double b = GetTrueAnomaly(end_orbit, epoch) + end_orbit.w;

    // Get the delta phase angle so that we can match up
    // Make sure that the phase angle matches up
    HohmannTransfer(start_orbit, end_orbit.semi_major_axis);
    // Get the new semi major axis
    double new_sma = (start_orbit.semi_major_axis + end_orbit.semi_major_axis) / 2;
    double t_trans = PI * sqrt((new_sma * new_sma * new_sma) * start_orbit.GM);
    // So calculate the phase angle that we have to do
    // Get mean motion of both bodies
    double phase_angle = PI - sqrt(end_orbit.GM / end_orbit.semi_major_axis) * t_trans;
    // Get the time it will be at the phase angle

    return std::pair<glm::dvec3, double>();
}
}  // namespace cqsp::common::systems
