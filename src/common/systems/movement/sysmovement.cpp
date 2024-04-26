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
#include "common/components/movement.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/components/units.h"
#include "common/util/nameutil.h"

namespace components = cqsp::common::components;
namespace ships = components::ships;
namespace types = components::types;
namespace bodies = components::bodies;
namespace systems = cqsp::common::systems;
using types::Kinematics;
using types::Orbit;
using bodies::Body;
using bodies::OrbitalSystem;
using types::Impulse;
using types::SurfaceCoordinate;
using bodies::DirtyOrbit;
using components::CommandQueue;
using systems::SysSurface;
using systems::SysPath;
using systems::SysOrbit;
using entt::entity;

void SysOrbit::DoSystem() {
    ZoneScoped;
    Universe& universe = GetGame().GetUniverse();
    ParseOrbitTree(entt::null, universe.sun);
}

namespace cqsp::common::systems {

void LeaveSOI(Universe& universe, const entity& body, entity& parent, Orbit& orb, Kinematics& pos, Kinematics& p_pos) {
    // Then change parent, then set the orbit
    auto& p_orb = universe.get<Orbit>(parent);
    if (p_orb.reference_body == entt::null) {
        return;
    }
    // Then add to orbital system
    universe.get<OrbitalSystem>(p_orb.reference_body).push_back(body);

    auto& parent_parent_orb = universe.get<Body>(p_orb.reference_body);

    auto& pp_pos = universe.get<Kinematics>(p_orb.reference_body);
    // Remove from parent
    auto& pt = universe.get<OrbitalSystem>(parent);
    std::erase(pt.children, body);
    // Get velocity and change posiiton
    // Convert orbit
    orb = types::Vec3ToOrbit(pos.position + p_pos.position, pos.velocity + p_pos.velocity, parent_parent_orb.GM,
                             universe.date.ToSecond());
    orb.reference_body = p_orb.reference_body;

    // Update dirty orbit
    universe.emplace_or_replace<DirtyOrbit>(body);
}

/// <summary>
/// Check if the entity has crashed into its parent object
/// </summary>
/// <param name="universe"></param>
/// <param name="orb"></param>
/// <param name="body"></param>
/// <param name="parent"></param>
void CrashObject(Universe& universe, Orbit& orb, entity body, entity parent) {
    if (universe.any_of<Body>(parent)) {
        return;
    }
    auto& p_bod = universe.get<Body>(parent);
    auto& pos = universe.get<Kinematics>(body);
    if (universe.any_of<ships::Crash>(body)) {
        pos.position = glm::vec3(0);
    }

    // Next time we need to account for the atmosphere
    if (glm::length(pos.position) <= p_bod.radius) {
        // Crash
        SPDLOG_INFO("Object {} collided with the ground", body);
        // Then remove from the tree or something like that
        universe.get_or_emplace<components::ships::Crash>(body);
        pos.position = glm::vec3(0);
        orb.semi_major_axis = 0;
    }
}

void CalculateImpulse(Universe& universe, Orbit& orb, entity body, entity parent) {
    if (!universe.any_of<Impulse>(body)) {
        return;
    }
    // Then add to the orbit the speed.
    // Then also convert the velocity
    Impulse& impulse = universe.get<Impulse>(body);
    auto reference = orb.reference_body;
    auto& pos = universe.get_or_emplace<Kinematics>(body);

    orb = types::Vec3ToOrbit(pos.position, pos.velocity + impulse.impulse, orb.GM, universe.date.ToSecond());
    orb.reference_body = reference;
    pos.position = types::toVec3(orb);
    pos.velocity = OrbitVelocityToVec3(orb, orb.v);
    universe.emplace_or_replace<DirtyOrbit>(body);
    // Remove impulse
    universe.remove<Impulse>(body);
}

void UpdateCommandQueue(Universe& universe, Orbit& orb, entity body, entity parent) {
    // Process thrust before updating orbit
    if (!universe.any_of<CommandQueue>(body)) {
        return;
    }
    // Check if the current date is beyond the universe date
    auto& queue = universe.get<CommandQueue>(body);
    if (queue.commands.empty()) {
        return;
    }
    auto& command = queue.commands.front();
    if (command.time > universe.date.ToSecond()) {
        return;
    }
    // Then execute the command
    orb = types::ApplyImpulse(orb, command.delta_v, command.time);
    universe.emplace_or_replace<DirtyOrbit>(body);
    queue.commands.pop_front();
}

bool EnterSOI(Universe& universe, const entity& parent, const entity& body) {
    // We should ignore bodies
    if (universe.any_of<Body>(body)) {
        return false;
    }
    SPDLOG_TRACE("Calculating SOI entrance for {} in {}", util::GetName(universe, body),
                 util::GetName(universe, parent));

    auto& pos = universe.get<Kinematics>(body);
    auto& orb = universe.get<Orbit>(body);
    // Check parents for SOI if we're inters ecting with anything
    auto& o_system = universe.get<OrbitalSystem>(parent);

    for (entt::entity child : o_system.children) {
        // Get the stuff
        if (child == body) {
            continue;
        }
        // Check the distance
        if (!universe.all_of<Body, Kinematics>(child)) {
            continue;
        }
        const Body& body_comp = universe.get<Body>(child);
        const auto& kinematics = universe.get<Kinematics>(child);
        if (glm::distance(kinematics.position, pos.position) <= body_comp.SOI) {
            // Calculate position
            orb = types::Vec3ToOrbit(pos.position - kinematics.position, pos.velocity - kinematics.velocity,
                                     body_comp.GM, universe.date.ToSecond());
            orb.reference_body = child;
            // Calculate position, and change the thing
            pos.position = types::toVec3(orb);
            pos.velocity = types::OrbitVelocityToVec3(orb, orb.v);
            // Then change SOI
            universe.get_or_emplace<OrbitalSystem>(child).push_back(body);
            auto& vec = universe.get<OrbitalSystem>(parent).children;
            vec.erase(std::remove(vec.begin(), vec.end(), body), vec.end());
            universe.emplace_or_replace<DirtyOrbit>(body);
            return true;
        }
        // Now check if it's intersecting with any things outside of stuff
        if (parent == universe.sun) {
            continue;
        }
    }
    return false;
}
}  // namespace cqsp::common::systems

void SysOrbit::ParseOrbitTree(entity parent, entity body) {
    Universe& universe = GetGame().GetUniverse();
    if (!universe.valid(body)) {
        return;
    }

    auto& orb = universe.get<Orbit>(body);

    UpdateCommandQueue(universe, orb, body, parent);

    types::UpdateOrbit(orb, universe.date.ToSecond());
    auto& pos = universe.get_or_emplace<Kinematics>(body);
    pos.position = toVec3(orb);
    pos.velocity = OrbitVelocityToVec3(orb, orb.v);

    if (parent != entt::null) {
        auto& p_pos = universe.get_or_emplace<Kinematics>(parent);
        // If distance is above SOI, then be annoyed
        auto& p_bod = universe.get<Body>(parent);
        if (glm::length(pos.position) > p_bod.SOI) {
            LeaveSOI(universe, body, parent, orb, pos, p_pos);
        }

        CrashObject(universe, orb, body, parent);

        CalculateImpulse(universe, orb, body, parent);
        pos.center = p_pos.center + p_pos.position;
        if (EnterSOI(universe, parent, body)) {
            SPDLOG_INFO("Entered SOI");
        }
    }

    auto& future_pos = universe.get_or_emplace<types::FuturePosition>(body);
    future_pos.position = types::OrbitTimeToVec3(orb, universe.date.ToSecond() + components::StarDate::TIME_INCREMENT);
    future_pos.center = pos.center;

    if (!universe.any_of<OrbitalSystem>(body)) {
        return;
    }
    for (entity entity : universe.get<OrbitalSystem>(body).children) {
        // Calculate position
        ParseOrbitTree(body, entity);
    }
}

void SysSurface::DoSystem() {
    Universe& universe = GetGame().GetUniverse();

    auto objects = universe.view<SurfaceCoordinate>();
    // First put them in a tree
    // Find all the entities
    for (entity object : objects) {
        SurfaceCoordinate& surface = universe.get<SurfaceCoordinate>(object);
        //cqspt::Kinematics& surfacekin = universe.get_or_emplace<cqspt::Kinematics>(object);
        //cqspt::Kinematics& center = universe.get<cqspt::Kinematics>(surface.planet);
        glm::vec3 anglevec = types::toVec3(surface);
        // Get planet radius
        //surfacekin.position = (anglevec * surface.radius + center.position);
    }
}

void SysPath::DoSystem() {
    ZoneScoped;
    Universe& universe = GetUniverse();

    auto bodies = universe.view<types::MoveTarget, Kinematics>(entt::exclude<Orbit>);
    for (entity body : bodies) {
        Kinematics& bodykin = universe.get<Kinematics>(body);
        Kinematics& targetkin = universe.get<Kinematics>(universe.get<types::MoveTarget>(body).target);
        glm::vec3 path = targetkin.position - bodykin.position;
    }
}
