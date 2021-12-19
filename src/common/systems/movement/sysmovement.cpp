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

void cqsp::common::systems::SysOrbit::DoSystem(Game& game) {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    Universe& universe = game.GetUniverse();

    auto bodies = universe.view<cqspt::Orbit>();
    for (entt::entity body : bodies) {
        auto& orb = universe.get<cqspt::Orbit>(body);
        cqspt::UpdateOrbit(orb);
        cqspt::UpdatePos(universe.get<cqspt::Kinematics>(body), orb);
    }
}

int cqsp::common::systems::SysOrbit::Interval() { return 1; }

void cqsp::common::systems::SysSurface::DoSystem(Game& game) {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;

    Universe& universe = game.GetUniverse();

    auto objects = universe.view<cqspt::SurfaceCoordinate>();
    for (entt::entity object : objects) {
        cqspt::SurfaceCoordinate& surface = universe.get<cqspt::SurfaceCoordinate>(object);
        //cqspt::Kinematics& surfacekin = universe.get_or_emplace<cqspt::Kinematics>(object);
        //cqspt::Kinematics& center = universe.get<cqspt::Kinematics>(surface.planet);
        glm::vec3 anglevec =
            glm::vec3(cos(surface.latitude) * cos(surface.longitude),
                      cos(surface.latitude) * sin(surface.longitude),
                      sin(surface.latitude));
        // Get planet radius
        //surfacekin.position = (anglevec * surface.radius + center.position);
    }
}

int cqsp::common::systems::SysSurface::Interval() {
    return 1;
}

void cqsp::common::systems::SysPath::DoSystem(Game& game) {
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    Universe& universe = game.GetUniverse();

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

int cqsp::common::systems::SysPath::Interval() { return 1; }
