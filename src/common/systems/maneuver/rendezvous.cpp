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

namespace components = cqsp::common::components;
namespace types = components::types;

namespace cqsp::common::systems {

HohmannPair_t CoplanarIntercept(const components::types::Orbit& start_orbit, const components::types::Orbit& end_orbit,
                                double epoch) {
    // They need to be the same plane, but let's ignore that
    // Also needs to be circular
    double current_phase_angle = CalculatePhaseAngle(start_orbit, end_orbit, epoch);

    double t_phase_angle = CalculateTransferAngle(start_orbit, end_orbit);

    double t_wait = -(t_phase_angle - current_phase_angle) / (start_orbit.nu() - end_orbit.nu());
    // Get the nearesxt time to that phase angle, maybe next time we can put a time where we can
    // Get the delta phase angle so that we can match up
    // Make sure that the phase angle matches up

    auto maneuver = UnsafeHohmannTransfer(start_orbit, end_orbit.semi_major_axis);

    // This is the initial burn
    maneuver.first.second += t_wait;
    // This is the subsequent burn
    maneuver.second.second += t_wait;

    return maneuver;
}
}  // namespace cqsp::common::systems
