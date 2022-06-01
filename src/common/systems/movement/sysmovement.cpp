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

#include "common/components/ships.h"
#include "common/components/coordinates.h"
#include "common/components/units.h"
#include <spdlog/spdlog.h>

namespace cqsp::common::systems {
void SysOrbit::DoSystem() {
    Universe& universe = GetGame().GetUniverse();
    ParseOrbitTree(entt::null, universe.sun);
}

void SysOrbit::ParseOrbitTree(entt::entity parent, entt::entity body) {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    Universe& universe = GetGame().GetUniverse();
    // Calculate the position
    auto [orb, body_comp] = universe.get<cqspt::Orbit, cqspc::bodies::Body>(body);
    cqspt::UpdateOrbit(orb, universe.date.ToSecond());
    auto& pos = universe.get_or_emplace<cqspt::Kinematics>(body);
    pos.position = cqspt::toVec3(orb);

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
                // Remove from parent
                auto& pt = universe.get<cqspc::bodies::OrbitalSystem>(parent);
                std::erase(pt.children, body);
                // Get velocity and change posiiton
                SPDLOG_INFO("Removed object");
            }
        }
        pos.position += p_pos.position;
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

int SysSurface::Interval() {
    return 1;
}

void SysPath::DoSystem() {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    Universe& universe = GetUniverse();

    auto bodies = universe.view<cqspt::MoveTarget, cqspt::Kinematics>(entt::exclude<cqspt::Orbit>);
    for (entt::entity body : bodies) {
        cqspt::Kinematics& bodykin = universe.get<cqspt::Kinematics>(body);
        cqspt::Kinematics& targetkin = universe.get<cqspt::Kinematics>(universe.get<cqspt::MoveTarget>(body).target);
        glm::vec3 path = targetkin.position - bodykin.position;
        if (glm::length(path) < bodykin.topspeed) {
            bodykin.position = targetkin.position;
        } else {
            bodykin.position += (targetkin.topspeed * glm::normalize(path));
        }
    }
}

int SysPath::Interval() { return 1; }
}  // namespace cqsp::common::systems
