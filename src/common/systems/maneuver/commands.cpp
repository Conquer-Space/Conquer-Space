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

#include "common/components/movement.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/maneuver/rendezvous.h"

namespace cqsp::common::systems::commands {
bool VerifyCommand(Universe& universe, entt::entity command) {
    return (command == entt::null || !universe.any_of<Trigger>(command) || !universe.any_of<Command>(command));
}

void ExecuteCommand(Universe& universe, entt::entity entity, entt::entity command_entity, Command command) {
    // TODO(EhWhoAmI): What if there's an error with the command?
    if (!universe.any_of<components::types::Orbit>(entity)) {
        return;
    }
    auto& orbit = universe.get<components::types::Orbit>(entity);

    // One huge switch statement is not how I want it to be but what can I do ¯\_(ツ)_/¯
    switch (command) {
        case Command::CircularizeAtPeriapsis: {
            std::pair<glm::dvec3, double> man_t = CircularizeAtPeriapsis(orbit);
            PushManeuvers(universe, entity, {man_t});
        } break;
        case Command::CircularizeAtApoapsis: {
            std::pair<glm::dvec3, double> man_t = CircularizeAtApoapsis(orbit);
            PushManeuvers(universe, entity, {man_t});
        } break;
        case Command::MatchPlanes: {
            if (!universe.any_of<OrbitTarget>(command_entity)) {
                break;
            }
            auto& target_orbit = universe.get<OrbitTarget>(command_entity);
            std::pair<glm::dvec3, double> man_t = MatchPlanes(orbit, target_orbit.orbit);
            PushManeuvers(universe, entity, {man_t});
        } break;
        case Command::CoplanarIntercept: {
            if (!universe.any_of<OrbitTarget>(command_entity)) {
                break;
            }
            auto& target_orbit = universe.get<OrbitTarget>(command_entity);
            auto pair = cqsp::common::systems::CoplanarIntercept(orbit, target_orbit.orbit, universe.date());
            PushManeuvers(universe, entity, {pair.first});
        } break;
        case Command::CoplanarInterceptAndTransfer: {
            if (!universe.any_of<OrbitTarget>(command_entity)) {
                break;
            }
            auto& target_orbit = universe.get<OrbitTarget>(command_entity);
            auto pair = cqsp::common::systems::CoplanarIntercept(orbit, target_orbit.orbit, universe.date());
            PushManeuvers(universe, entity, {pair.first, pair.second});
        } break;
        case Command::SetInclination: {
            if (!universe.any_of<OrbitScalar>(command_entity)) {
                break;
            }
            auto& scalar_change = universe.get<OrbitScalar>(command_entity);
            auto maneuver = SetInclination(orbit, scalar_change.value);
            PushManeuvers(universe, entity, {maneuver});
        } break;
        case Command::SetApoapsis: {
            if (!universe.any_of<OrbitScalar>(command_entity)) {
                break;
            }
            auto& scalar_change = universe.get<OrbitScalar>(command_entity);
            auto maneuver = SetApoapsis(orbit, scalar_change.value);
            PushManeuvers(universe, entity, {maneuver});
        } break;
        case Command::SetPeriapsis: {
            if (!universe.any_of<OrbitScalar>(command_entity)) {
                break;
            }
            auto& scalar_change = universe.get<OrbitScalar>(command_entity);
            auto maneuver = SetPeriapsis(orbit, scalar_change.value);
            PushManeuvers(universe, entity, {maneuver});
        } break;
        default:
            break;
    }
}

bool ProcessCommandQueue(Universe& universe, entt::entity body, Trigger trigger) {
    if (!universe.any_of<components::CommandQueue>(body)) {
        return false;
    }

    auto& queue = universe.get<components::CommandQueue>(body);
    if (queue.commands.empty()) {
        return false;  // empty command queue
    }

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
    components::types::Orbit current_orbit = universe.get<components::types::Orbit>(agent);

    auto maneuver = cqsp::common::systems::MatchPlanes(current_orbit, target_orbit);
    PushManeuvers(universe, agent, {maneuver});

    // Move to intercept
    entt::entity maneuver_to_point = universe.create();
    universe.emplace<Trigger>(maneuver_to_point, Trigger::OnManeuver);
    universe.emplace<Command>(maneuver_to_point, Command::CoplanarIntercept);
    universe.emplace<OrbitTarget>(maneuver_to_point, target_orbit);

    // Circularize around target body
    entt::entity circularize = universe.create();
    universe.emplace<Trigger>(circularize, Trigger::OnEnterSOI);
    universe.emplace<Command>(circularize, Command::CircularizeAtPeriapsis);

    auto& command_queue = universe.get_or_emplace<components::CommandQueue>(agent);
    command_queue.commands.push_back(maneuver_to_point);
    command_queue.commands.push_back(circularize);
}

void PushManeuvers(Universe& universe, entt::entity entity, std::initializer_list<components::Maneuver_t> maneuver,
                   double offset) {
    // Now push back all the commands or something
    auto& queue = universe.get_or_emplace<components::CommandQueue>(entity);
    for (auto& man_t : maneuver) {
        queue.maneuvers.emplace_back(components::Maneuver(man_t, universe.date() + offset));
    }
}

void PushManeuvers(Universe& universe, entt::entity entity, components::HohmannPair_t hohmann_pair, double offset) {
    auto& queue = universe.get_or_emplace<components::CommandQueue>(entity);
    queue.maneuvers.emplace_back(components::Maneuver(hohmann_pair.first, universe.date() + offset));
    queue.maneuvers.emplace_back(components::Maneuver(hohmann_pair.second, universe.date() + offset));
}
}  // namespace cqsp::common::systems::commands
