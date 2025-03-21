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

#include "common/components/movement.h"
#include "common/components/orbit.h"
#include "common/universe.h"

namespace cqsp::common::systems::commands {
enum class Command {
    CirculaizeAtApoapsis,
    CircularizeAtPeriapsis,
    SetApoapsis,
    SetPeriapsis,
    SetInclination,
    MatchPlanes,
    CoplanarIntercept,             // Just intercepts
    CoplanarInterceptAndTransfer,  // Intercepts and matches orbit
    Impulse,
};

enum class Trigger {
    // When to trigger the command
    OnEnterSOI,
    OnExitSOI,
    OnManeuver,  // When the previous manuever is executed
    ASAP,
};

struct OrbitTarget {
    components::types::Orbit orbit;
};

/// Verifies if the command is a valid ship command option.
/// This verifies if it's not null, and contains the
/// necessary components to make it a command
bool VerifyCommand(Universe& universe, entt::entity command);
/// Returns true if a command is processed and popped, returns false if not
bool ProcessCommandQueue(Universe& universe, entt::entity body, Trigger trigger);
void ExecuteCommand(Universe& universe, entt::entity entity, entt::entity command_entity, Command command);
void TransferToMoon(Universe& universe, entt::entity agent, entt::entity target);
}  // namespace cqsp::common::systems::commands
