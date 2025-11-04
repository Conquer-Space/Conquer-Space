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
#include "common/components/movement.h"
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
    ParseOrbitTree(entt::null, GetUniverse().sun);
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

    auto& pp_pos = GetUniverse().get<Kinematics>(p_orb.reference_body);
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

bool SysOrbit::CrashObject(Orbit& orb, entt::entity body, entt::entity parent) {
    ZoneScoped;
    if (GetUniverse().any_of<Body>(body)) {
        return false;
    }
    auto& p_bod = GetUniverse().get<Body>(parent);
    auto& pos = GetUniverse().get<Kinematics>(body);
    if (GetUniverse().any_of<ships::Crash>(body)) {
        pos.position = glm::dvec3(0);
        // Also clear the command queue or something
        if (GetUniverse().any_of<components::CommandQueue>(body)) {
            GetUniverse().remove<components::CommandQueue>(body);
        }
        return true;
    }

    // Next time we need to account for the atmosphere
    if (glm::length(pos.position) > p_bod.radius) {
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
void SysOrbit::CalculateImpulse(types::Orbit& orb, entt::entity body, entt::entity parent) {
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
    for (entt::entity entity : GetUniverse().get<components::bodies::OrbitalSystem>(body).children) {
        // Calculate position
        ParseOrbitTree(body, entity);
    }
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
        SPDLOG_INFO("Negative time? {}", GetUniverse().date.ToSecond() - command.time);
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
    auto& orb = GetUniverse().get<types::Orbit>(body);

    {
        ZoneScopedN("Orbit Update");
        types::UpdateOrbit(orb, GetUniverse().date.ToSecond());
    }
    UpdateCommandQueue(orb, body, parent);

    auto& pos = GetUniverse().get_or_emplace<types::Kinematics>(body);

    // Set our current true anomaly for debugging purposes
    if (GetUniverse().any_of<types::SetTrueAnomaly>(body)) {
        orb.v = GetUniverse().get<types::SetTrueAnomaly>(body).true_anomaly;
        // Set new mean anomaly at epoch
        GetUniverse().remove<types::SetTrueAnomaly>(body);
    }
    {
        ZoneScopedN("Position determination");
        pos.position = types::toVec3(orb);
        pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);
    }
    glm::dvec3 future_center = glm::dvec3(0, 0, 0);
    if (parent != entt::null) {
        ZoneScopedN("Future Position computation");
        auto& p_pos = GetUniverse().get_or_emplace<types::Kinematics>(parent);
        // If distance is above SOI, then be annoyed
        auto& p_bod = GetUniverse().get<components::bodies::Body>(parent);
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
            if (GetUniverse().any_of<types::FuturePosition>(parent)) {
                auto& future_pos = GetUniverse().get<types::FuturePosition>(parent);
                future_center = future_pos.center + future_pos.position;
            }
        }
        if (EnterSOI(parent, body)) {
            //auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
            //pause_opt.to_tick = false;
            SPDLOG_INFO("Entered SOI");
        }
    }

    // If they're crashed then we don't care about the future position
    auto& future_pos = GetUniverse().get_or_emplace<types::FuturePosition>(body);
    future_pos.position =
        types::OrbitTimeToVec3(orb, GetUniverse().date.ToSecond() + components::StarDate::TIME_INCREMENT);
    future_pos.center = future_center;

    ParseChildren(body);
}

bool SysOrbit::EnterSOI(const entt::entity& parent, const entt::entity& body) {
    ZoneScoped;
    // We should ignore bodies
    if (GetUniverse().any_of<Body>(body)) {
        return false;
    }
    SPDLOG_TRACE("Calculating SOI entrance for {} in {}", util::GetName(universe, body),
                 util::GetName(universe, parent));

    if (!GetUniverse().all_of<Kinematics, Orbit>(body)) {
        return false;
    }
    auto& pos = GetUniverse().get<Kinematics>(body);
    auto& orb = GetUniverse().get<Orbit>(body);
    // Check parents for SOI if we're intersecting with anything
    auto& o_system = GetUniverse().get<OrbitalSystem>(parent);

    for (entt::entity entity : o_system.bodies) {
        // Get the stuff
        if (entity == body) {
            continue;
        }

        const auto& body_comp = GetUniverse().get<Body>(entity);
        const auto& target_position = GetUniverse().get<Kinematics>(entity);
        if (glm::distance(target_position.position, pos.position) <= body_comp.SOI) {
            // Calculate position
            if (debug_prints) {
                SPDLOG_INFO("Pre enter position: {}", glm::to_string(pos.position - target_position.position));
                SPDLOG_INFO("Pre enter velocity: {}", glm::to_string(pos.velocity - target_position.velocity));
            }

            orb = types::Vec3ToOrbit(pos.position - target_position.position, pos.velocity - target_position.velocity,
                                     body_comp.GM, GetUniverse().date.ToSecond());
            if (debug_prints) {
                SPDLOG_INFO("Post enter SOI maneuver: {}", orb.ToHumanString());
            }
            orb.reference_body = entity;
            // Calculate position, and change the thing
            pos.position = types::toVec3(orb);
            pos.center = target_position.position + target_position.center;
            pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);

            if (debug_prints) {
                SPDLOG_INFO("Post enter position: {}", glm::to_string(pos.position));
                SPDLOG_INFO("Post enter velocity: {}", glm::to_string(pos.velocity));
            }

            // Then change SOI
            GetUniverse().get_or_emplace<OrbitalSystem>(entity).push_back(body);
            auto& vec = GetUniverse().get<OrbitalSystem>(parent).children;
            vec.erase(std::remove(vec.begin(), vec.end(), body), vec.end());
            GetUniverse().emplace_or_replace<bodies::DirtyOrbit>(body);
            // I have a bad feeling about this
            commands::ProcessCommandQueue(GetUniverse(), body, components::Trigger::OnEnterSOI);
            return true;
        }
        // Now check if it's intersecting with any things outside of stuff
        if (parent == GetUniverse().sun) {
            continue;
        }
    }
    return false;
}
}  // namespace cqsp::common::systems
