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
#include "common/actions/maneuver/commands.h"

#include <spdlog/spdlog.h>

#include <algorithm>

#include <tracy/Tracy.hpp>

#include "common/actions/maneuver/basicmaneuver.h"
#include "common/actions/maneuver/rendezvous.h"
#include "common/actions/maneuver/transfers.h"
#include "common/components/maneuver.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"

namespace cqsp::common::systems::commands {

namespace types = components::types;

using components::Command;
using components::OrbitEntityTarget;
using components::OrbitScalar;
using components::OrbitTarget;
using components::Trigger;
using types::Orbit;

bool VerifyCommand(Node& command) {
    return (command == entt::null || !command.any_of<Trigger>() || !command.any_of<Command>());
}

void ExecuteCommand(Node& orbit_node, Node& command_node, Command command) {
    // TODO(EhWhoAmI): What if there's an error with the command?
    if (!orbit_node.any_of<Orbit>()) {
        return;
    }
    Orbit& orbit = orbit_node.get<Orbit>();

    // One huge switch statement is not how I want it to be but what can I do ¯\_(ツ)_/¯
    switch (command) {
        case Command::CircularizeAtPeriapsis: {
            std::pair<glm::dvec3, double> man_t = CircularizeAtPeriapsis(orbit);
            SPDLOG_INFO("Circularizing at periapsis");
            PushManeuvers(orbit_node, {man_t});
        } break;
        case Command::CircularizeAtApoapsis: {
            std::pair<glm::dvec3, double> man_t = CircularizeAtApoapsis(orbit);
            SPDLOG_INFO("Circularizing at apoapsis");
            PushManeuvers(orbit_node, {man_t});
        } break;
        case Command::MatchPlanes: {
            if (!command_node.any_of<OrbitTarget>()) {
                break;
            }
            auto& target_orbit = command_node.get<OrbitTarget>();
            std::pair<glm::dvec3, double> man_t = MatchPlanes(orbit, target_orbit.orbit);
            SPDLOG_INFO("Matching plane");
            PushManeuvers(orbit_node, {man_t});
        } break;
        case Command::CoplanarIntercept: {
            if (!command_node.any_of<OrbitTarget>()) {
                break;
            }
            auto& target_orbit = command_node.get<OrbitTarget>();
            auto pair = CoplanarIntercept(orbit, target_orbit.orbit, orbit_node.universe().date());
            SPDLOG_INFO("Coplanar intercepting");
            PushManeuvers(orbit_node, {pair.first});
        } break;
        case Command::CoplanarInterceptAndTransfer: {
            if (!command_node.any_of<OrbitTarget>()) {
                break;
            }
            auto& target_orbit = command_node.get<OrbitTarget>();
            SPDLOG_INFO("Coplanar intercept and transfer");
            auto pair = CoplanarIntercept(orbit, target_orbit.orbit, orbit_node.universe().date());
            PushManeuvers(orbit_node, {pair.first, pair.second});
        } break;
        case Command::SetInclination: {
            if (!command_node.any_of<OrbitScalar>()) {
                break;
            }
            auto& scalar_change = command_node.get<OrbitScalar>();
            SPDLOG_INFO("Setting Inclination to {}", components::types::toDegree(scalar_change.value));
            auto maneuver = SetInclination(orbit, scalar_change.value);
            PushManeuvers(orbit_node, {maneuver});
        } break;
        case Command::SetApoapsis: {
            if (!command_node.any_of<OrbitScalar>()) {
                break;
            }
            auto& scalar_change = command_node.get<OrbitScalar>();
            SPDLOG_INFO("Setting apoapsis to {}", scalar_change.value);
            auto maneuver = SetApoapsis(orbit, scalar_change.value);
            PushManeuvers(orbit_node, {maneuver});
        } break;
        case Command::SetPeriapsis: {
            if (!command_node.any_of<OrbitScalar>()) {
                break;
            }
            auto& scalar_change = command_node.get<OrbitScalar>();
            SPDLOG_INFO("Setting Periapsis to {}", scalar_change.value);
            auto maneuver = SetPeriapsis(orbit, scalar_change.value);
            PushManeuvers(orbit_node, {maneuver});
        } break;
        case Command::LandOnBody: {
            // Lands on body when crashed
            // Then check if it's going to
            if (!command_node.any_of<OrbitEntityTarget>()) {
                break;
            }
            Node target_city(command_node, command_node.get<OrbitEntityTarget>().target);
            // Then check if it's the same body as the body that we landed on
            // Eh who cares
            auto& docked_ships = target_city.get_or_emplace<components::DockedShips>();
            // Now add the entity back
            docked_ships.docked_ships.emplace_back(orbit_node);
            // Also remove from the orbital system
            // Get the current orbital system that we're in
            Orbit& orbit = orbit_node.get<Orbit>();
            Node reference_body = Node(orbit_node, orbit.reference_body);
            if (reference_body != entt::null && reference_body.valid() &&
                reference_body.any_of<components::bodies::OrbitalSystem>()) {
                auto& children = reference_body.get<components::bodies::OrbitalSystem>().children;
                children.erase(std::remove(children.begin(), children.end(), orbit_node), children.end());
            }
            SPDLOG_INFO("Landing on city {}", util::GetName(target_city));
            orbit_node.remove<Orbit>();
            orbit_node.remove<types::Kinematics>();
        } break;
        case Command::ForceMatchPlanes: {
            if (!command_node.any_of<OrbitTarget>()) {
                break;
            }
            auto& target_orbit = command_node.get<OrbitTarget>().orbit;
            auto& orbit = orbit_node.get<Orbit>();

            orbit.LAN = target_orbit.LAN;
            orbit.inclination = target_orbit.inclination;
            // Add a maneuver. This is a hack so that we run the next maneuver command after this
            // TODO(EhWhoAmI): Fix this when we are able to figure out why plane matching doesn't work as well as we would hope.
            PushManeuver(orbit_node, MakeManeuver(glm::dvec3(0, 0, 0), 100.));
            SPDLOG_INFO("Forced match plane");
        }
        case Command::InterceptAndCircularizeBody: {
            // We should intercept the body and stuff
        } break;
        case Command::ExitSOI: {
            if (!command_node.any_of<OrbitScalar>()) {
                break;
            }
            auto& scalar_change = command_node.get<OrbitScalar>();
            auto& orbit = orbit_node.get<Orbit>();
            auto maneuver = common::systems::TransferFromBody(orbit_node.universe(), orbit,
                                                              orbit_node.get<types::Kinematics>(),
                                                              scalar_change.value);
            PushManeuver(orbit_node, maneuver);
        } break;
        case Command::SelfDestruct: {
            // Self destruct
            orbit_node.emplace_or_replace<components::ships::Crash>();
        } break;
        default:
            break;
    }
}

bool ProcessCommandQueue(Node& body, Trigger trigger) {
    ZoneScoped;
    if (!body.any_of<components::CommandQueue>()) {
        return false;
    }

    auto& queue = body.get<components::CommandQueue>();
    if (queue.commands.empty()) {
        return false;  // empty command queue
    }

    Node next_command(body, queue.commands.front());
    if (commands::VerifyCommand(next_command)) {
        // Command is just bugged
        queue.commands.pop_front();
        return false;
    }

    if (next_command.get<Trigger>() != trigger) {
        return false;  // Didn't match trigger, execute command later
    }

    // Now execute the command
    commands::ExecuteCommand(body, next_command, next_command.get<commands::Command>());

    // Done executing the command, then we're done
    next_command.destroy();
    queue.commands.pop_front();
    return true;
}

void TransferToMoon(Node& agent, Node& target) {
    // Now generate the commands
    // Match planes
    // Get orbit
    Orbit target_orbit = target.get<Orbit>();
    Orbit current_orbit = agent.get<Orbit>();
    auto& command_queue = agent.get_or_emplace<components::CommandQueue>();

    if (current_orbit.eccentricity > 0.00001) {
        // Then we should circularize
        auto maneuver2 = CircularizeAtPeriapsis(current_orbit);
        PushManeuver(agent, maneuver2);

        Node match_plane(agent.universe());
        match_plane.emplace<Trigger>(Trigger::OnManeuver);
        match_plane.emplace<Command>(Command::MatchPlanes);
        match_plane.emplace<OrbitTarget>(target_orbit);
        command_queue.commands.push_back(match_plane);

        Node force_match_plane(agent.universe());
        force_match_plane.emplace<Trigger>(Trigger::OnManeuver);
        force_match_plane.emplace<Command>(Command::ForceMatchPlanes);
        force_match_plane.emplace<OrbitTarget>(target_orbit);
        command_queue.commands.push_back(force_match_plane);
    } else {
        auto maneuver = MatchPlanes(current_orbit, target_orbit);
        PushManeuvers(agent, {maneuver});
        Node force_match_plane(agent.universe());
        force_match_plane.emplace<Trigger>(Trigger::OnManeuver);
        force_match_plane.emplace<Command>(Command::ForceMatchPlanes);
        force_match_plane.emplace<OrbitTarget>(target_orbit);
        command_queue.commands.push_back(force_match_plane);
    }

    // Move to intercept
    Node maneuver_to_point(agent.universe());
    maneuver_to_point.emplace<Trigger>(Trigger::OnManeuver);
    maneuver_to_point.emplace<Command>(Command::CoplanarIntercept);
    maneuver_to_point.emplace<OrbitTarget>(target_orbit);

    // Circularize around target body
    Node circularize(agent.universe());
    circularize.emplace<Trigger>(Trigger::OnEnterSOI);
    circularize.emplace<Command>(Command::CircularizeAtPeriapsis);

    command_queue.commands.push_back(maneuver_to_point);
    command_queue.commands.push_back(circularize);
}

/**
 * @param offset the time offset in ticks to push back the command
 */
void PushManeuver(Node& entity, components::Maneuver_t maneuver, double offset) {
    auto& queue = entity.get_or_emplace<components::CommandQueue>();
    queue.maneuvers.emplace_back(maneuver, entity.universe().date() + offset);
}

void PushManeuvers(Node& entity, std::initializer_list<components::Maneuver_t> maneuver,
                   double offset) {
    // Now push back all the commands or something
    auto& queue = entity.get_or_emplace<components::CommandQueue>();
    for (auto& man_t : maneuver) {
        queue.maneuvers.emplace_back(man_t, entity.universe().date() + offset);
    }
}

void PushManeuvers(Node& entity, components::HohmannPair_t hohmann_pair, double offset) {
    auto& queue = entity.get_or_emplace<components::CommandQueue>();
    queue.maneuvers.emplace_back(hohmann_pair.first, entity.universe().date() + offset);
    queue.maneuvers.emplace_back(hohmann_pair.second, entity.universe().date() + offset);
}

void LandOnMoon(Node& agent, Node& target, Node& city) {
    // If the current body is the moon, then we don't really need to bother
    auto& orbit = agent.get<components::types::Orbit>();
    if (orbit.reference_body != target) {
        TransferToMoon(agent, target);
    }
    // Lower the periapsis to about 90% of the radius
    // In theory we'll do the math to land on the moon properly but eh
    auto& body = target.get<components::bodies::Body>();
    double landing_radius = body.radius * 0.9;
    Node land_action(agent.universe());
    land_action.emplace<Trigger>(Trigger::OnManeuver);
    land_action.emplace<Command>(Command::SetPeriapsis);
    land_action.emplace<OrbitScalar>(landing_radius);

    // then land on the city on the moon
    Node dock_city(agent.universe());
    dock_city.emplace<Trigger>(Trigger::OnCrash);
    dock_city.emplace<Command>(Command::LandOnBody);
    dock_city.emplace<OrbitEntityTarget>(city);

    auto& command_queue = agent.get_or_emplace<components::CommandQueue>();
    command_queue.commands.push_back(land_action);
    command_queue.commands.push_back(dock_city);
}

components::Maneuver_t MakeManeuver(const glm::dvec3& vector, double time) { return std::make_pair(vector, time); }

std::vector<Node> GetSOIHierarchy(Node& source) {
    std::vector<Node> source_list;
    Node parent_body(source, source.get<Orbit>().reference_body);
    source_list.push_back(source);

    while (parent_body != entt::null) {
        source_list.push_back(parent_body);
        parent_body = Node(source, parent_body.get<Orbit>().reference_body);
    }
    return source_list;
}

Node GetCommonSOI(Node& source, Node& target) {
    // Get common ancestor
    std::vector<Node> source_list = GetSOIHierarchy(source);
    std::vector<Node> target_list = GetSOIHierarchy(target);
    size_t i;
    for (i = 1; i < std::min(source_list.size(), target_list.size()); i++) {
        if (source_list[source_list.size() - i] != target_list[target_list.size() - i]) {
            return source_list[source_list.size() - i + 1];
        }
    }
    if (i == std::min(source_list.size(), target_list.size())) {
        return source_list[source_list.size() - i];
    }
    return source_list[source_list.size() - 1];
}

void LeaveSOI(Node& agent, double altitude) {
    Node escape_action(agent.universe());
    escape_action.emplace<Trigger>(Trigger::OnManeuver);
    escape_action.emplace<Command>(Command::ExitSOI);
    escape_action.emplace<OrbitScalar>(altitude);

    auto& command_queue = agent.get_or_emplace<components::CommandQueue>();
    command_queue.commands.push_back(escape_action);
}
}  // namespace cqsp::common::systems::commands
