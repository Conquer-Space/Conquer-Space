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
#include "core/systems/movement/sysorbit.h"

#include <cmath>
#include <vector>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <tracy/Tracy.hpp>

#include "core/actions/maneuver/commands.h"
#include "core/components/coordinates.h"
#include "core/components/maneuver.h"
#include "core/components/orbit.h"
#include "core/components/ships.h"
#include "core/components/units.h"
#include "core/util/nameutil.h"

namespace cqsp::core::systems {
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
    // Let's parse the bodies
    for (auto&& [entity, orbit, body, kinematics, future_pos] :
         universe.view<Orbit, Body, Kinematics, types::FuturePosition>().each()) {
        types::UpdateOrbit(orbit, GetUniverse().date.ToSecond());
        kinematics.position = types::toVec3(orbit);
        kinematics.velocity = types::OrbitVelocityToVec3(orbit, orbit.v);
        body_storage[entity] = std::make_pair(kinematics.position, body.SOI);
        future_pos.position =
            types::OrbitTimeToVec3(orbit, GetUniverse().date.ToSecond() + components::StarDate::TIME_INCREMENT);
        future_position_storage[entity] = future_pos.position;
    }

    // now compute our hierachy
    ComputeCenters(GetUniverse().sun, glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0));

    ParseChildren(GetUniverse().sun);
}

void SysOrbit::LeaveSOI(const entt::entity& body, entt::entity& parent, Orbit& orb, Kinematics& pos,
                        Kinematics& p_pos) {
    ZoneScoped;
    // Then change parent, then set the orbit
    auto& p_orb = GetUniverse().get<Orbit>(parent);
    if (p_orb.reference_body == entt::null) {
        return;
    }
    // Then add to orbital system
    GetUniverse().get<OrbitalSystem>(p_orb.reference_body).push_back(body);

    auto& parent_parent_orb = GetUniverse().get<Body>(p_orb.reference_body);

    const auto& pp_pos = GetUniverse().get<Kinematics>(p_orb.reference_body);
    // Remove from parent
    auto& pt = GetUniverse().get<OrbitalSystem>(parent);
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

    commands::ProcessCommandQueue(GetUniverse(), body, components::Trigger::OnExitSOI);
}

bool SysOrbit::CrashObject(Orbit& orb, entt::entity body, Kinematics& pos, double radius) {
    ZoneScoped;
    if (GetUniverse().any_of<Body>(body)) {
        return false;
    }
    if (GetUniverse().any_of<ships::Crash>(body)) {
        pos.position = glm::dvec3(0);
        // Also clear the command queue or something
        if (GetUniverse().any_of<components::CommandQueue>(body)) {
            GetUniverse().remove<components::CommandQueue>(body);
        }
        return true;
    }

    // Next time we need to account for the atmosphere
    if (glm::length(pos.position) > radius) {
        return false;
    }
    // Check if there is a command
    if (commands::ProcessCommandQueue(GetUniverse(), body, components::Trigger::OnCrash)) {
        SPDLOG_INFO("Executed command on crash");
    } else {
        // Crash
        SPDLOG_INFO("Object {} collided with the ground", util::GetName(GetUniverse(), body));
        // Then remove from the tree or something like that
        GetUniverse().get_or_emplace<ships::Crash>(body);
        GetUniverse().get_or_emplace<bodies::DirtyOrbit>(body);
        pos.position = glm::dvec3(0);
        orb.semi_major_axis = 0;
    }
    return true;
}

/**
 * Adds an impulse in the body centered inertial frame
 */
void SysOrbit::CalculateImpulse(types::Orbit& orb, entt::entity body) {
    ZoneScoped;
    if (!GetUniverse().any_of<types::Impulse>(body)) {
        return;
    }
    // Then add to the orbit the speed.
    // Then also convert the velocity
    auto& impulse = GetUniverse().get<types::Impulse>(body);
    auto reference = orb.reference_body;
    auto& pos = GetUniverse().get_or_emplace<Kinematics>(body);

    orb = types::Vec3ToOrbit(pos.position, pos.velocity + impulse.impulse, orb.GM, GetUniverse().date.ToSecond());
    orb.reference_body = reference;
    pos.position = types::toVec3(orb);
    pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);
    GetUniverse().emplace_or_replace<bodies::DirtyOrbit>(body);
    // Remove impulse
    GetUniverse().remove<types::Impulse>(body);
}

void SysOrbit::ParseChildren(entt::entity body) {
    ZoneScoped;
    if (!GetUniverse().any_of<components::bodies::OrbitalSystem>(body)) {
        return;
    }
    auto& orbital_system = GetUniverse().get<components::bodies::OrbitalSystem>(body);
    for (entt::entity entity : orbital_system.bodies) {
        // We should shortcircuit and get our children lol
        ParseChildren(entity);
    }
    // Parse the child bodies
    for (entt::entity entity : orbital_system.children) {
        // Calculate position
        ParseOrbitTree(body, entity);
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

void SysOrbit::UpdateCommandQueue(Orbit& orb, entt::entity body, entt::entity parent) {
    ZoneScoped;
    // Process thrust before updating orbit
    if (!GetUniverse().any_of<components::CommandQueue>(body)) {
        return;
    }
    // Check if the current date is beyond the universe date
    auto& queue = GetUniverse().get<components::CommandQueue>(body);
    if (queue.empty()) {
        return;
    }
    auto& command = queue.maneuvers.front();
    if (command.time > GetUniverse().date.ToSecond()) {
        return;
    }
    if (GetUniverse().date.ToSecond() - command.time > Interval()) {
        SPDLOG_TRACE("Negative maneuver time? {}", GetUniverse().date.ToSecond() - command.time);
    }
    if (GetUniverse().any_of<components::ships::Crash>(body)) {
        // If crashed
        return;
    }
    // Then execute the command
    orb = types::ApplyImpulse(orb, command.delta_v, command.time);
    // Get the difference in orbit

    GetUniverse().emplace_or_replace<components::bodies::DirtyOrbit>(body);
    // Check if the next command is something, and then execute it
    queue.maneuvers.pop_front();
    // Now then executethe next command or something like that
    // Then check the command queue for more commands
    commands::ProcessCommandQueue(GetUniverse(), body, components::Trigger::OnManeuver);
}

void SysOrbit::ParseOrbitTree(entt::entity parent, entt::entity body) {
    ZoneScoped;
    if (!GetUniverse().valid(body)) {
        return;
    }
    // Check if the body has crashed
    if (GetUniverse().any_of<ships::Crash>(body)) {
        return;
    }
    if (!GetUniverse().all_of<types::Orbit>(body)) {
        SPDLOG_INFO("{} {}", core::util::GetName(GetUniverse(), body), body);
        return;
    }
    ComputePosition(parent, body);
}

void SysOrbit::ComputePosition(entt::entity parent, entt::entity body) {
    ZoneScoped;
    auto& orb = GetUniverse().get<types::Orbit>(body);

    {
        ZoneScopedN("Orbit Update");
        types::UpdateOrbit(orb, GetUniverse().date.ToSecond());
    }
    UpdateCommandQueue(orb, body, parent);

    auto& future_pos = GetUniverse().get_or_emplace<types::FuturePosition>(body);
    auto& pos = GetUniverse().get_or_emplace<types::Kinematics>(body);
    {
        ZoneScopedN("Position determination");
        pos.position = types::toVec3(orb);
        pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);
    }
    glm::dvec3 future_center = glm::dvec3(0, 0, 0);
    CalculateImpulse(orb, body);
    if (parent != entt::null) {
        ZoneScopedN("Future Position computation");
        // If distance is above SOI, then be annoyed
        double SOI = body_storage[parent].second;
        if (glm::length(pos.position) > SOI) {
            auto& p_bod = GetUniverse().get<components::bodies::Body>(parent);
            auto& p_pos = GetUniverse().get_or_emplace<types::Kinematics>(parent);
            LeaveSOI(body, parent, orb, pos, p_pos);
        }

        if (CrashObject(orb, body, pos, 100)) {
            return;
        }

        pos.center = center_storage[parent];

        {
            ZoneScopedN("Computing future position");
            future_center = center_storage[parent];
        }
        if (CheckEnterSOI(parent, body, pos)) {
            SPDLOG_INFO("Entered SOI");
        }
    }

    // If they're crashed then we don't care about the future position
    future_pos.position =
        types::OrbitTimeToVec3(orb, GetUniverse().date.ToSecond() + components::StarDate::TIME_INCREMENT);
    future_pos.center = future_center;
}

bool SysOrbit::CheckEnterSOI(const entt::entity& parent, const entt::entity& body, Kinematics& pos) {
    ZoneScoped;
    // We should ignore bodies
    if (GetUniverse().any_of<Body>(body)) {
        return false;
    }
    SPDLOG_TRACE("Calculating SOI entrance for {} in {}", util::GetName(universe, body),
                 util::GetName(universe, parent));

    // Check parents for SOI if we're intersecting with anything
    auto& o_system = GetUniverse().get<OrbitalSystem>(parent);

    for (entt::entity entity : o_system.bodies) {
        // Get the stuff
        if (entity == body) {
            continue;
        }

        const auto& [target_pos, SOI] = body_storage[entity];
        if (glm::distance(target_pos, pos.position) > SOI) {
            continue;
        }
        auto& orb = GetUniverse().get<Orbit>(body);
        const auto& body_comp = GetUniverse().get<Body>(entity);
        const auto& target_position = GetUniverse().get<Kinematics>(entity);
        EnterSOI(entity, body, parent, orb, pos, body_comp, target_position);

        // I have a bad feeling about this
        commands::ProcessCommandQueue(GetUniverse(), body, components::Trigger::OnEnterSOI);
        return true;
    }
    return false;
}

void SysOrbit::EnterSOI(entt::entity entity, entt::entity body, entt::entity parent, Orbit& orb,
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
    GetUniverse().get_or_emplace<OrbitalSystem>(entity).push_back(body);
    auto& vec = GetUniverse().get<OrbitalSystem>(parent).children;
    vec.erase(std::remove(vec.begin(), vec.end(), body), vec.end());
    GetUniverse().emplace_or_replace<bodies::DirtyOrbit>(body);
}

void SysOrbit::ComputeCenters(entt::entity entity, glm::dvec3 parent_pos, glm::dvec3 future_parent_pos) {
    auto& system = GetUniverse().get<OrbitalSystem>(entity);
    for (const entt::entity child : system.bodies) {
        glm::dvec3 pos = body_storage[child].first + parent_pos;
        glm::dvec3 future_pos = future_position_storage[child] + future_parent_pos;
        center_storage[child] = pos;
        future_center_storage[child] = future_pos;
        ComputeCenters(child, pos, future_pos);
        GetUniverse().get<Kinematics>(child).center = parent_pos;
        GetUniverse().get<types::FuturePosition>(child).center = future_parent_pos;
    }
}

void SysOrbit::Init() {
    for (entt::entity entity : GetUniverse().view<Orbit, Body, Kinematics>()) {
        GetUniverse().emplace<types::FuturePosition>(entity);
    }
}
}  // namespace cqsp::core::systems
