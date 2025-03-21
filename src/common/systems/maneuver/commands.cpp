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
#include "common/systems/maneuver/commands.h"

#include "common/systems/maneuver/maneuver.h"

namespace cqsp::common::systems::commands {
bool VerifyCommand(Universe& universe, entt::entity command) {
    return (command == entt::null || !universe.any_of<Trigger>(command) || !universe.any_of<Command>(command));
}

void ExecuteCommand(Universe& universe, entt::entity entity, entt::entity command_entity, Command command) {
    // TODO(EhWhoAmI): What if there's an error with the command?
}

bool ProcessCommandQueue(Universe& universe, entt::entity body, Trigger trigger) {
    if (!universe.any_of<components::CommandQueue>(body)) {
        return;
    }

    auto& queue = universe.get<components::CommandQueue>(body);

    entt::entity next_command = queue.commands.front();
    if (commands::VerifyCommand(universe, next_command)) {
        // Command is just bugged
        queue.commands.pop_front();
        return false;
    }

    if (universe.get<Trigger>(next_command) != trigger) {
        return false;  // Didn't match trigger, execute command later
    }

    // Now execute the command
    commands::ExecuteCommand(universe, body, next_command, universe.get<commands::Command>(next_command));
    // Done executing the command, then we're done
    queue.commands.pop_front();
    return true;
}

void TransferToMoon(Universe& universe, entt::entity agent, entt::entity target) {
    // Now generate the commands
    // Match planes
    // Get orbit
    components::types::Orbit target_orbit = universe.get<components::types::Orbit>(target);
    entt::entity plane_match = universe.create();
    universe.emplace<Trigger>(plane_match, Trigger::ASAP);
    universe.emplace<Command>(plane_match, Command::MatchPlanes);
    universe.emplace<OrbitTarget>(plane_match, target_orbit);

    // Move to intercept
    entt::entity maneuver_to_point = universe.create();
    universe.emplace<Trigger>(plane_match, Trigger::OnManeuver);
    universe.emplace<Command>(plane_match, Command::CoplanarIntercept);
    universe.emplace<OrbitTarget>(plane_match, target_orbit);

    // Circularize around target body
    entt::entity circularize = universe.create();
    universe.emplace<Trigger>(circularize, Trigger::OnEnterSOI);
    universe.emplace<Command>(circularize, Command::CircularizeAtPeriapsis);

    auto& command_queue = universe.get_or_emplace<components::CommandQueue>(agent);
    command_queue.commands.push_back(plane_match);
    command_queue.commands.push_back(maneuver_to_point);
    command_queue.commands.push_back(circularize);
}
}  // namespace cqsp::common::systems::commands
