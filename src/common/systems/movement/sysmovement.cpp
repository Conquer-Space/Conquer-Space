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
#include "common/systems/movement/sysmovement.h"

#include <cmath>
#include <vector>

#include <tracy/Tracy.hpp>

#include "common/components/coordinates.h"
#include "common/components/movement.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/components/units.h"
#include "common/systems/maneuver/commands.h"
#include "common/util/nameutil.h"

namespace cqsp::common::systems {
namespace cqspc = cqsp::common::components;
namespace cqsps = cqsp::common::components::ships;
namespace cqspt = cqsp::common::components::types;

void SysOrbit::DoSystem() {
    ZoneScoped;
    Universe& universe = GetGame().GetUniverse();
    ParseOrbitTree(entt::null, GetUniverse().sun);
}

void SysOrbit::LeaveSOI(const entt::entity& body, entt::entity& parent, cqspt::Orbit& orb, cqspt::Kinematics& pos,
                        cqspt::Kinematics& p_pos) {
    // Then change parent, then set the orbit
    auto& p_orb = GetUniverse().get<cqspt::Orbit>(parent);
    if (p_orb.reference_body == entt::null) {
        return;
    }
    // Then add to orbital system
    GetUniverse().get<cqspc::bodies::OrbitalSystem>(p_orb.reference_body).push_back(body);

    auto& parent_parent_orb = GetUniverse().get<cqspc::bodies::Body>(p_orb.reference_body);

    auto& pp_pos = GetUniverse().get<cqspt::Kinematics>(p_orb.reference_body);
    // Remove from parent
    auto& pt = GetUniverse().get<cqspc::bodies::OrbitalSystem>(parent);
    std::erase(pt.children, body);
    // Get velocity and change posiiton
    // Convert orbit
    orb = cqspt::Vec3ToOrbit(pos.position + p_pos.position, pos.velocity + p_pos.velocity, parent_parent_orb.GM,
                             GetUniverse().date.ToSecond());
    orb.reference_body = p_orb.reference_body;

    // Set new position
    pos.position = pos.position + p_pos.position;
    pos.velocity = pos.velocity + p_pos.velocity;
    // Update dirty orbit
    GetUniverse().emplace_or_replace<cqspc::bodies::DirtyOrbit>(body);

    commands::ProcessCommandQueue(GetUniverse(), body, commands::Trigger::OnExitSOI);
}

void SysOrbit::CrashObject(cqspt::Orbit& orb, entt::entity body, entt::entity parent) {
    if (GetUniverse().any_of<cqspc::bodies::Body>(body)) {
        return;
    }
    auto& p_bod = GetUniverse().get<cqspc::bodies::Body>(parent);
    auto& pos = GetUniverse().get<cqspt::Kinematics>(body);
    if (GetUniverse().any_of<cqsps::Crash>(body)) {
        pos.position = glm::vec3(0);
    }

    // Next time we need to account for the atmosphere
    if (glm::length(pos.position) <= p_bod.radius) {
        // Crash
        SPDLOG_INFO("Object {} collided with the ground", (uint64_t)body);
        // Then remove from the tree or something like that
        GetUniverse().get_or_emplace<cqsps::Crash>(body);
        pos.position = glm::vec3(0);
        orb.semi_major_axis = 0;
    }
}

void SysOrbit::CalculateImpulse(cqspt::Orbit& orb, entt::entity body, entt::entity parent) {
    if (GetUniverse().any_of<cqspc::types::Impulse>(body)) {
        // Then add to the orbit the speed.
        // Then also convert the velocity
        auto& impulse = GetUniverse().get<cqspc::types::Impulse>(body);
        auto reference = orb.reference_body;
        auto& pos = GetUniverse().get_or_emplace<cqspt::Kinematics>(body);

        orb = cqspt::Vec3ToOrbit(pos.position, pos.velocity + impulse.impulse, orb.GM, GetUniverse().date.ToSecond());
        orb.reference_body = reference;
        pos.position = cqspt::toVec3(orb);
        pos.velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
        GetUniverse().emplace_or_replace<cqspc::bodies::DirtyOrbit>(body);
        // Remove impulse
        GetUniverse().remove<cqspc::types::Impulse>(body);
    }
}

void SysOrbit::UpdateCommandQueue(cqspt::Orbit& orb, entt::entity body, entt::entity parent) {
    // Process thrust before updating orbit
    if (!GetUniverse().any_of<cqspc::CommandQueue>(body)) {
        return;
    }
    // Check if the current date is beyond the universe date
    auto& queue = GetUniverse().get<cqspc::CommandQueue>(body);
    if (queue.maneuvers.empty()) {
        return;
    }
    auto& command = queue.maneuvers.front();
    if (command.time > GetUniverse().date.ToSecond()) {
        return;
    }
    if (GetUniverse().date.ToSecond() - command.time > Interval()) {
        SPDLOG_INFO("Negative time? {}", GetUniverse().date.ToSecond() - command.time);
    }
    // Then execute the command
    orb = cqspt::ApplyImpulse(orb, command.delta_v, command.time);
    GetUniverse().emplace_or_replace<cqspc::bodies::DirtyOrbit>(body);
    // Check if the next command is something, and then execute it
    queue.maneuvers.pop_front();
    // Now then executethe next command or something like that
    // Then check the command queue for more commands
    commands::ProcessCommandQueue(GetUniverse(), body, commands::Trigger::OnManeuver);
}

void SysOrbit::ParseOrbitTree(entt::entity parent, entt::entity body) {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    if (!GetUniverse().valid(body)) {
        return;
    }

    auto& orb = GetUniverse().get<cqspt::Orbit>(body);

    UpdateCommandQueue(orb, body, parent);

    cqspt::UpdateOrbit(orb, GetUniverse().date.ToSecond());
    auto& pos = GetUniverse().get_or_emplace<cqspt::Kinematics>(body);
    if (GetUniverse().any_of<cqspt::SetTrueAnomaly>(body)) {
        orb.v = GetUniverse().get<cqspt::SetTrueAnomaly>(body).true_anomaly;
        // Set new mean anomaly at epoch
        GetUniverse().remove<cqspt::SetTrueAnomaly>(body);
    }
    pos.position = cqspt::toVec3(orb);
    pos.velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);

    if (parent != entt::null) {
        auto& p_pos = GetUniverse().get_or_emplace<cqspt::Kinematics>(parent);
        // If distance is above SOI, then be annoyed
        auto& p_bod = GetUniverse().get<cqspc::bodies::Body>(parent);
        if (glm::length(pos.position) > p_bod.SOI) {
            LeaveSOI(body, parent, orb, pos, p_pos);
        }

        CrashObject(orb, body, parent);

        CalculateImpulse(orb, body, parent);
        pos.center = p_pos.center + p_pos.position;
        if (EnterSOI(parent, body)) {
            SPDLOG_INFO("Entered SOI");
        }
    }

    auto& future_pos = GetUniverse().get_or_emplace<cqspt::FuturePosition>(body);
    future_pos.position =
        cqspt::OrbitTimeToVec3(orb, GetUniverse().date.ToSecond() + components::StarDate::TIME_INCREMENT);
    future_pos.center = pos.center;

    if (!GetUniverse().any_of<cqspc::bodies::OrbitalSystem>(body)) {
        return;
    }
    for (entt::entity entity : GetUniverse().get<cqspc::bodies::OrbitalSystem>(body).children) {
        // Calculate position
        ParseOrbitTree(body, entity);
    }
}

void SysSurface::DoSystem() {
    Universe& universe = GetGame().GetUniverse();

    auto objects = GetUniverse().view<cqspt::SurfaceCoordinate>();
    // First put them in a tree
    // Find all the entities
    for (entt::entity object : objects) {
        cqspt::SurfaceCoordinate& surface = GetUniverse().get<cqspt::SurfaceCoordinate>(object);
        //cqspt::Kinematics& surfacekin = universe.get_or_emplace<cqspt::Kinematics>(object);
        //cqspt::Kinematics& center = universe.get<cqspt::Kinematics>(surface.planet);
        glm::vec3 anglevec = cqspt::toVec3(surface);
        // Get planet radius
        //surfacekin.position = (anglevec * surface.radius + center.position);
    }
}

void SysPath::DoSystem() {
    ZoneScoped;
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    Universe& universe = GetUniverse();

    auto bodies = GetUniverse().view<cqspt::MoveTarget, cqspt::Kinematics>(entt::exclude<cqspt::Orbit>);
    for (entt::entity body : bodies) {
        cqspt::Kinematics& bodykin = GetUniverse().get<cqspt::Kinematics>(body);
        cqspt::Kinematics& targetkin =
            GetUniverse().get<cqspt::Kinematics>(GetUniverse().get<cqspt::MoveTarget>(body).target);
        glm::vec3 path = targetkin.position - bodykin.position;
    }
}

bool SysOrbit::EnterSOI(const entt::entity& parent, const entt::entity& body) {
    // We should ignore bodies
    if (GetUniverse().any_of<cqspc::bodies::Body>(body)) {
        return false;
    }
    SPDLOG_TRACE("Calculating SOI entrance for {} in {}", util::GetName(universe, body),
                 util::GetName(universe, parent));

    auto& pos = GetUniverse().get<cqspc::types::Kinematics>(body);
    auto& orb = GetUniverse().get<cqspc::types::Orbit>(body);
    // Check parents for SOI if we're intersecting with anything
    auto& o_system = GetUniverse().get<cqspc::bodies::OrbitalSystem>(parent);

    for (entt::entity entity : o_system.children) {
        // Get the stuff
        if (entity == body) {
            continue;
        }
        // Check the distance
        if (!GetUniverse().all_of<cqspc::bodies::Body, cqspc::types::Kinematics>(entity)) {
            continue;
        }
        const auto& body_comp = GetUniverse().get<cqspc::bodies::Body>(entity);
        const auto& target_position = GetUniverse().get<cqspc::types::Kinematics>(entity);
        if (glm::distance(target_position.position, pos.position) <= body_comp.SOI) {
            // Calculate position
            orb = cqspt::Vec3ToOrbit(pos.position - target_position.position, pos.velocity - target_position.velocity,
                                     body_comp.GM, GetUniverse().date.ToSecond());
            orb.reference_body = entity;
            // Calculate position, and change the thing
            pos.position = cqspt::toVec3(orb);
            pos.velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
            // Then change SOI
            GetUniverse().get_or_emplace<cqspc::bodies::OrbitalSystem>(entity).push_back(body);
            auto& vec = GetUniverse().get<cqspc::bodies::OrbitalSystem>(parent).children;
            vec.erase(std::remove(vec.begin(), vec.end(), body), vec.end());
            GetUniverse().emplace_or_replace<cqspc::bodies::DirtyOrbit>(body);
            // I have a bad feeling about this
            commands::ProcessCommandQueue(GetUniverse(), body, commands::Trigger::OnEnterSOI);
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
