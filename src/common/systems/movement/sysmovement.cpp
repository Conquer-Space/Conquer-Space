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
#include "common/systems/movement/sysmovement.h"

#include <cmath>

#include <tracy/Tracy.hpp>

#include "common/components/coordinates.h"
#include "common/components/ships.h"
#include "common/components/units.h"

namespace cqsp::common::systems {
namespace cqspc = cqsp::common::components;
namespace cqsps = cqsp::common::components::ships;
namespace cqspt = cqsp::common::components::types;

void SysOrbit::DoSystem() {
    ZoneScoped;
    Universe& universe = GetGame().GetUniverse();
    ParseOrbitTree(entt::null, universe.sun);
}

void LeaveSOI(Universe& universe, const entt::entity& body, entt::entity& parent, cqspt::Orbit& orb,
              cqspt::Kinematics& pos, cqspt::Kinematics& p_pos) {
    // Then change parent, then set the orbit
    auto& p_orb = universe.get<cqspt::Orbit>(parent);
    if (p_orb.reference_body != entt::null) {
        return;
    }
    // Then add to orbital system
    universe.get<cqspc::bodies::OrbitalSystem>(p_orb.reference_body).push_back(body);

    auto& parent_parent_orb = universe.get<cqspc::bodies::Body>(p_orb.reference_body);

    auto& pp_pos = universe.get<cqspt::Kinematics>(p_orb.reference_body);
    // Remove from parent
    auto& pt = universe.get<cqspc::bodies::OrbitalSystem>(parent);
    std::erase(pt.children, body);
    // Get velocity and change posiiton
    // Convert orbit
    orb = cqspt::Vec3ToOrbit(pos.position + p_pos.position, pos.velocity + p_pos.velocity, parent_parent_orb.GM,
                             universe.date.ToSecond());
    orb.reference_body = p_orb.reference_body;
    orb.CalculateVariables();

    // Update dirty orbit
    universe.emplace_or_replace<cqspc::bodies::DirtyOrbit>(body);
}

void SysOrbit::ParseOrbitTree(entt::entity parent, entt::entity body) {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    Universe& universe = GetGame().GetUniverse();
    if (!universe.valid(body)) {
        return;
    }

    // Calculate the position
    auto& orb = universe.get<cqspt::Orbit>(body);
    cqspt::UpdateOrbit(orb, universe.date.ToSecond());
    auto& pos = universe.get_or_emplace<cqspt::Kinematics>(body);
    pos.position = cqspt::toVec3(orb);
    pos.velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);

    if (parent != entt::null) {
        auto& p_pos = universe.get_or_emplace<cqspt::Kinematics>(parent);
        // If distance is above SOI, then be annoyed
        auto& p_bod = universe.get<cqspc::bodies::Body>(parent);
        if (glm::length(pos.position) > p_bod.SOI) {
            LeaveSOI(universe, body, parent, orb, pos, p_pos);
        }

        if (universe.any_of<cqsps::Crash>(body)) {
            pos.position = glm::vec3(0);
        }
        // Next time we need to account for the atmosphere
        if (glm::length(pos.position) <= p_bod.radius) {
            // Crash
            SPDLOG_INFO("Object {} collided with the ground", body);
            // Then remove from the tree or something like that
            universe.get_or_emplace<cqsps::Crash>(body);
            pos.position = glm::vec3(0);
            orb.semi_major_axis = 0;
        }

        if (universe.any_of<cqspc::types::Impulse>(body)) {
            // Then add to the orbit the speed.
            // Then also convert the velocity
            auto& impulse = universe.get<cqspc::types::Impulse>(body);
            auto reference = orb.reference_body;

            orb = cqspt::Vec3ToOrbit(pos.position, pos.velocity + impulse.impulse, p_bod.GM, universe.date.ToSecond());
            orb.reference_body = reference;
            orb.CalculateVariables();
            pos.position = cqspt::toVec3(orb);
            pos.velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
            universe.emplace_or_replace<cqspc::bodies::DirtyOrbit>(body);
            // Remove impulse
            universe.remove<cqspc::types::Impulse>(body);
        }
        pos.center = p_pos.center + p_pos.position;
        EnterSOI(universe, parent, body);
    }

    auto& future_pos = universe.get_or_emplace<cqspt::FuturePosition>(body);
    future_pos.position = cqspt::OrbitTimeToVec3(orb, universe.date.ToSecond() + components::StarDate::TIME_INCREMENT);
    future_pos.center = pos.center;

    if (!universe.any_of<cqspc::bodies::OrbitalSystem>(body)) {
        return;
    }
    for (entt::entity entity : universe.get<cqspc::bodies::OrbitalSystem>(body).children) {
        // Calculate position
        ParseOrbitTree(body, entity);
    }
}

void SysSurface::DoSystem() {
    Universe& universe = GetGame().GetUniverse();

    auto objects = universe.view<cqspt::SurfaceCoordinate>();
    // First put them in a tree
    // Find all the entities
    for (entt::entity object : objects) {
        cqspt::SurfaceCoordinate& surface = universe.get<cqspt::SurfaceCoordinate>(object);
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

    auto bodies = universe.view<cqspt::MoveTarget, cqspt::Kinematics>(entt::exclude<cqspt::Orbit>);
    for (entt::entity body : bodies) {
        cqspt::Kinematics& bodykin = universe.get<cqspt::Kinematics>(body);
        cqspt::Kinematics& targetkin = universe.get<cqspt::Kinematics>(universe.get<cqspt::MoveTarget>(body).target);
        glm::vec3 path = targetkin.position - bodykin.position;
    }
}

void EnterSOI(Universe& universe, const entt::entity& parent, const entt::entity& body) {
    auto& pos = universe.get<cqspc::types::Kinematics>(body);
    auto& orb = universe.get<cqspc::types::Orbit>(body);
    // Check parents for SOI if we're intersecting with anything
    for (entt::entity entity : universe.get<cqspc::bodies::OrbitalSystem>(parent).children) {
        // Get the stuff
        if (entity == body) {
            continue;
        }
        // Check the distance
        if (!universe.all_of<cqspc::bodies::Body, cqspc::types::Kinematics>(entity)) {
            continue;
        }
        const auto& body_comp = universe.get<cqspc::bodies::Body>(entity);
        const auto& kinematics = universe.get<cqspc::types::Kinematics>(entity);
        if (glm::distance(kinematics.position, pos.position) <= body_comp.radius) {
            // Calculate position
            orb = cqspt::Vec3ToOrbit(pos.position - kinematics.position, pos.velocity - kinematics.velocity,
                                     body_comp.GM, universe.date.ToSecond());
            orb.reference_body = entity;
            orb.CalculateVariables();
            // Calculate position, and change the thing
            pos.position = cqspt::toVec3(orb);
            pos.velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
            // Then change SOI
            universe.get_or_emplace<cqspc::bodies::OrbitalSystem>(entity).push_back(body);
            auto& vec = universe.get<cqspc::bodies::OrbitalSystem>(parent).children;
            vec.erase(std::remove(vec.begin(), vec.end(), body), vec.end());
            break;
        }
    }
}
}  // namespace cqsp::common::systems
