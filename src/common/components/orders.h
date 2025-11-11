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

#include <entt/entt.hpp>

#include "common/components/orbit.h"

namespace cqsp::common::components {
enum class Command {
    CircularizeAtApoapsis,
    CircularizeAtPeriapsis,
    SetApoapsis,
    SetPeriapsis,
    SetInclination,
    MatchPlanes,
    CoplanarIntercept,             // Just intercepts
    CoplanarInterceptAndTransfer,  // Intercepts and matches orbit
    Impulse,
    LandOnBody,
    InterceptAndCircularizeBody,  // More generic command that will transition through spheres of influence
    ForceMatchPlanes,             // Matches planes immediately. Needs a corresponding orbit to match
    ExitSOI,
    SelfDestruct
};

enum class Trigger {
    // When to trigger the command
    OnEnterSOI,
    OnExitSOI,
    OnManeuver,  // When the previous manuever is executed
    OnCrash,
};

struct OrbitTarget {
    types::Orbit orbit;
};

struct OrbitScalar {
    double value;  // Modifies the orbit by some sort of scalar
};

struct OrbitEntityTarget {
    entt::entity target;
};
}  // namespace cqsp::common::components