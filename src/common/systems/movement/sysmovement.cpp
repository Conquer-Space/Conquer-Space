/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <math.h>

#include <tracy/Tracy.hpp>

#include "common/components/ships.h"
#include "common/components/coordinates.h"
#include "common/components/units.h"

namespace cqsp::common::systems {
void SysOrbit::DoSystem() {
    ZoneScoped;
    Universe& universe = GetGame().GetUniverse();
    ParseOrbitTree(entt::null, universe.sun);
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
            // Then change parent, then set the orbit
            auto& p_orb = universe.get<cqspt::Orbit>(parent);
            if (p_orb.reference_body != entt::null) {
                // Then add to orbital system
                universe.get<cqspc::bodies::OrbitalSystem>(p_orb.reference_body)
                    .push_back(body);

                auto& parent_parent_orb = universe.get<cqspc::bodies::Body>(p_orb.reference_body);
                if (p_orb.reference_body != entt::null) {
                    // Some stuff
                }
                auto& pp_pos =
                    universe.get<cqspt::Kinematics>(p_orb.reference_body);
                // Remove from parent
                auto& pt = universe.get<cqspc::bodies::OrbitalSystem>(parent);
                std::erase(pt.children, body);
                // Get velocity and change posiiton
                // Convert orbit
                orb = cqspt::Vec3ToOrbit(pos.position + p_pos.position,
                                         pos.velocity + p_pos.velocity,
                                         parent_parent_orb.GM, GetUniverse().date.ToSecond());
                orb.reference_body = p_orb.reference_body;
                orb.CalculateVariables();
            }
        } else {
        }
        pos.center = p_pos.center + p_pos.position;
    }

    if (!universe.any_of<cqspc::bodies::OrbitalSystem>(body)) {
        return;
    }
    for (entt::entity entity : universe.get<cqspc::bodies::OrbitalSystem>(body).children) {
        // Calculate position
        ParseOrbitTree(body, entity);
    }
}

void SysSurface::DoSystem() {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;

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

void LeaveSOI(Universe& universe, const entt::entity& body) {
    namespace cqspc = cqsp::common::components;
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspt = cqsp::common::components::types;
    // There are 2 bodies that are orbiting that matter, the currently orbiting object,
    // and the object that the current body is orbiting.
    auto& current_orbit = universe.get<cqspt::Orbit>(body);
    entt::entity parent = current_orbit.reference_body;
    if (parent == entt::null) {
        return;
    }
    auto& parent_orbit = universe.get<cqspt::Orbit>(parent);
    entt::entity greater_parent = parent_orbit.reference_body;
    if (greater_parent == entt::null) {
        return;
    }

    // Set reference body
    current_orbit.reference_body = greater_parent;

    // Current pos
    auto& pos = universe.get<cqspt::Kinematics>(body);
    auto& p_pos = universe.get<cqspt::Kinematics>(parent);

    auto& greater_body = universe.get<cqspb::Body>(greater_parent);
    // Calculate new orbit
    auto orb = cqspt::Vec3ToOrbit(pos.position + p_pos.position,
                                  pos.velocity + p_pos.velocity,
                                  greater_body.GM,
                           universe.date.ToSecond());
    orb.reference_body = greater_parent;
    orb.nu = greater_body.GM;
    orb.CalculateVariables();
    // Remove children
    auto& parent_orbital_system = universe.get<cqspb::OrbitalSystem>(parent);
    std::erase(parent_orbital_system.children, body);
    auto& greater_orbital_system = universe.get<cqspb::OrbitalSystem>(greater_parent);
    greater_orbital_system.push_back(body);
}
}  // namespace cqsp::common::systems
