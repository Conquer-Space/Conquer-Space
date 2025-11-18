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
#include "common/systems/movement/sysorbit.h"

#include <cmath>
#include <vector>

#include <glm/gtx/string_cast.hpp>
#include <tracy/Tracy.hpp>

#include "common/actions/maneuver/commands.h"
#include "common/components/coordinates.h"
#include "common/components/maneuver.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/components/units.h"
#include "common/util/nameutil.h"

namespace cqsp::common::systems {
namespace ships = components::ships;
namespace types = components::types;
namespace bodies = components::bodies;

using bodies::Body;
using bodies::OrbitalSystem;
using types::Kinematics;
using types::Orbit;

void SysOrbit::DoSystem() {
    ZoneScoped;
    Universe& universe = GetGame().GetUniverse();
    Node sun(universe, universe.sun);
    Node null_node(universe, entt::null);
    ParseOrbitTree(null_node, sun);
}

void SysOrbit::LeaveSOI(Node& body, Node& parent, Orbit& orb, Kinematics& pos,
                        Kinematics& p_pos) {
    ZoneScoped;
    // Then change parent, then set the orbit
    auto& p_orb = parent.get<Orbit>();
    if (p_orb.reference_body == entt::null) {
        return;
    }
    // Then add to orbital system
    Node grandparent(GetUniverse(), p_orb.reference_body);
    grandparent.get<OrbitalSystem>().push_back(body);

    auto& parent_parent_orb = grandparent.get<Body>();

    auto& pp_pos = grandparent.get<Kinematics>();
    // Remove from parent
    auto& pt = parent.get<OrbitalSystem>();
    std::erase(pt.children, body);
    // Get velocity and change posiiton
    // Convert orbit
    orb = types::Vec3ToOrbit(pos.position + p_pos.position, pos.velocity + p_pos.velocity, parent_parent_orb.GM,
                             GetUniverse().date.ToSecond());
    orb.reference_body = p_orb.reference_body;

    // Set new position
    pos.position = pos.position + p_pos.position;
    pos.velocity = pos.velocity + p_pos.velocity;
    // Update dirty orbit
    GetUniverse().emplace_or_replace<bodies::DirtyOrbit>(body);

    commands::ProcessCommandQueue(body, components::Trigger::OnExitSOI);
}

bool SysOrbit::CrashObject(Orbit& orb, Node& body, Node& parent) {
    ZoneScoped;
    if (body.any_of<Body>()) {
        return false;
    }
    auto& p_bod = parent.get<Body>();
    auto& pos = body.get<Kinematics>();
    if (body.any_of<ships::Crash>()) {
        pos.position = glm::dvec3(0);
        // Also clear the command queue or something
        if (body.any_of<components::CommandQueue>()) {
            body.remove<components::CommandQueue>();
        }
        return true;
    }

    // Next time we need to account for the atmosphere
    if (glm::length(pos.position) > p_bod.radius) {
        return false;
    }
    // Check if there is a command
    if (commands::ProcessCommandQueue(body, components::Trigger::OnCrash)) {
        SPDLOG_INFO("Executed command on crash");
    } else {
        // Crash
        SPDLOG_INFO("Object {} collided with the ground", util::GetName(body));
        // Then remove from the tree or something like that
        body.get_or_emplace<ships::Crash>();
        body.get_or_emplace<bodies::DirtyOrbit>();
        pos.position = glm::dvec3(0);
        orb.semi_major_axis = 0;
    }
    return true;
}

/**
 * Adds an impulse in the body centered inertial frame
 */
void SysOrbit::CalculateImpulse(types::Orbit& orb, Node& body, Node& parent) {
    ZoneScoped;
    if (!GetUniverse().any_of<types::Impulse>(body)) {
        return;
    }
    // Then add to the orbit the speed.
    // Then also convert the velocity
    auto& impulse = body.get<types::Impulse>();
    auto& pos = body.get_or_emplace<Kinematics>();

    orb = types::Vec3ToOrbit(pos.position, pos.velocity + impulse.impulse, orb.GM, GetUniverse().date.ToSecond());
    orb.reference_body = orb.reference_body;
    pos.position = types::toVec3(orb);
    pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);
    GetUniverse().emplace_or_replace<bodies::DirtyOrbit>(body);
    // Remove impulse
    GetUniverse().remove<types::Impulse>(body);
}

void SysOrbit::ParseChildren(Node& body) {
    ZoneScoped;
    if (!body.any_of<components::bodies::OrbitalSystem>()) {
        return;
    }
    auto& orbital_system = body.get<components::bodies::OrbitalSystem>();
    for (Node child_node : body.Convert(orbital_system.children)) {
        // Calculate position
        ParseOrbitTree(body, child_node);
    }
    // Now check if anything has crashed and remove
    orbital_system.children.erase(std::remove_if(orbital_system.children.begin(), orbital_system.children.end(),
                                                 [&](entt::entity entity) {
                                                     if (GetUniverse().any_of<ships::Crash>(entity)) {
                                                         GetUniverse().destroy(entity);
                                                         return true;
                                                     }

                                                     return false;
                                                 }),
                                  orbital_system.children.end());
}

void SysOrbit::UpdateCommandQueue(Orbit& orb, Node& body, Node& parent) {
    ZoneScoped;
    // Process thrust before updating orbit
    if (!body.any_of<components::CommandQueue>()) {
        return;
    }
    // Check if the current date is beyond the universe date
    auto& queue = body.get<components::CommandQueue>();
    if (queue.empty()) {
        return;
    }
    auto& command = queue.maneuvers.front();
    if (command.time > GetUniverse().date.ToSecond()) {
        return;
    }
    if (GetUniverse().date.ToSecond() - command.time > Interval()) {
        SPDLOG_INFO("Negative time? {}", GetUniverse().date.ToSecond() - command.time);
    }
    if (body.any_of<components::ships::Crash>()) {
        // If crashed
        return;
    }
    // Then execute the command
    orb = types::ApplyImpulse(orb, command.delta_v, command.time);
    // Get the difference in orbit

    body.emplace_or_replace<components::bodies::DirtyOrbit>();
    // Check if the next command is something, and then execute it
    queue.maneuvers.pop_front();
    // Now then executethe next command or something like that
    // Then check the command queue for more commands
    commands::ProcessCommandQueue(body, components::Trigger::OnManeuver);
}

void SysOrbit::ParseOrbitTree(Node& parent, Node& body) {
    ZoneScoped;

    if (!body.valid()) {
        return;
    }
    // Check if the body has crashed
    if (body.any_of<ships::Crash>()) {
        return;
    }
    auto& orb = body.get<types::Orbit>();

    {
        ZoneScopedN("Orbit Update");
        types::UpdateOrbit(orb, GetUniverse().date.ToSecond());
    }
    UpdateCommandQueue(orb, body, parent);

    auto& pos = body.get_or_emplace<types::Kinematics>();

    // Set our current true anomaly for debugging purposes
    if (body.any_of<types::SetTrueAnomaly>()) {
        orb.v = body.get<types::SetTrueAnomaly>().true_anomaly;
        // Set new mean anomaly at epoch
        body.remove<types::SetTrueAnomaly>();
    }
    {
        ZoneScopedN("Position determination");
        pos.position = types::toVec3(orb);
        pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);
    }
    glm::dvec3 future_center = glm::dvec3(0, 0, 0);
    if (parent != entt::null) {
        ZoneScopedN("Future Position computation");
        auto& p_pos = parent.get_or_emplace<types::Kinematics>();
        // If distance is above SOI, then be annoyed
        auto& p_bod = parent.get<components::bodies::Body>();
        if (glm::length(pos.position) > p_bod.SOI) {
            LeaveSOI(body, parent, orb, pos, p_pos);
        }

        // If it has crashed it is unlikely to have it's own orbital system, and even if it does
        // everything on that orbital system likely crashed as well
        if (CrashObject(orb, body, parent)) {
            return;
        }

        CalculateImpulse(orb, body, parent);
        pos.center = p_pos.center + p_pos.position;

        {
            ZoneScopedN("Computing future position");
            if (parent.any_of<types::FuturePosition>()) {
                auto& future_pos = GetUniverse().get<types::FuturePosition>(parent);
                future_center = future_pos.center + future_pos.position;
            }
        }
        if (CheckEnterSOI(parent, body)) {
            //auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
            //pause_opt.to_tick = false;
            SPDLOG_INFO("Entered SOI");
        }
    }

    // If they're crashed then we don't care about the future position
    auto& future_pos = body.get_or_emplace<types::FuturePosition>();
    future_pos.position =
        types::OrbitTimeToVec3(orb, GetUniverse().date.ToSecond() + components::StarDate::TIME_INCREMENT);
    future_pos.center = future_center;

    ParseChildren(body);
}

bool SysOrbit::CheckEnterSOI(Node& parent, Node& body) {
    ZoneScoped;
    // We should ignore bodies
    if (body.any_of<Body>()) {
        return false;
    }
    SPDLOG_TRACE("Calculating SOI entrance for {} in {}", util::GetName(body), util::GetName(parent));

    if (!body.all_of<Kinematics, Orbit>()) {
        return false;
    }
    auto& pos = body.get<Kinematics>();
    auto& orb = body.get<Orbit>();
    // Check parents for SOI if we're intersecting with anything
    auto& o_system = parent.get<OrbitalSystem>();

    for (Node node : parent.Convert(o_system.bodies)) {
        // Get the stuff
        if (node == body) {
            continue;
        }

        const Body& body_comp = node.get<Body>();
        const auto& target_position = node.get<Kinematics>();
        if (glm::distance(target_position.position, pos.position) > body_comp.SOI) {
            continue;
        }

        EnterSOI(node, body, parent, orb, pos, body_comp, target_position);

        // I have a bad feeling about this
        commands::ProcessCommandQueue(body, components::Trigger::OnEnterSOI);
        return true;
    }
    return false;
}

void SysOrbit::EnterSOI(Node& entity, Node& body, Node& parent, Orbit& orb,
                        Kinematics& vehicle_position, const Body& body_comp, const Kinematics& target_position) {
    ZoneScoped;
    // Calculate position
    if (debug_prints) {
        SPDLOG_INFO("Pre enter position: {}", glm::to_string(vehicle_position.position - target_position.position));
        SPDLOG_INFO("Pre enter velocity: {}", glm::to_string(vehicle_position.velocity - target_position.velocity));
    }

    orb = types::Vec3ToOrbit(vehicle_position.position - target_position.position,
                             vehicle_position.velocity - target_position.velocity, body_comp.GM,
                             GetUniverse().date.ToSecond());
    if (debug_prints) {
        SPDLOG_INFO("Post enter SOI maneuver: {}", orb.ToHumanString());
    }
    orb.reference_body = entity;
    // Calculate position, and change the thing
    vehicle_position.position = types::toVec3(orb);
    vehicle_position.center = target_position.position + target_position.center;
    vehicle_position.velocity = types::OrbitVelocityToVec3(orb, orb.v);

    if (debug_prints) {
        SPDLOG_INFO("Post enter position: {}", glm::to_string(vehicle_position.position));
        SPDLOG_INFO("Post enter velocity: {} ({})", glm::to_string(vehicle_position.velocity),
                    glm::length(vehicle_position.velocity));
    }

    // Then change SOI
    entity.get_or_emplace<OrbitalSystem>().push_back(body);
    auto& vec = parent.get<OrbitalSystem>().children;
    vec.erase(std::remove(vec.begin(), vec.end(), body), vec.end());
    body.emplace_or_replace<bodies::DirtyOrbit>(body);
}
}  // namespace cqsp::common::systems
