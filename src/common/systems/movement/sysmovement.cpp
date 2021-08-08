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
#include "sysmovement.h"

#include "common/components/ships.h"
#include "common/components/movement.h"
#include "common/components/units.h"

void conquerspace::common::systems::SysOrbit::DoSystem(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;

    auto bodies = universe.view<cqspt::Orbit>();
    for (entt::entity body : bodies) {
        // Disable orbits for now
        //auto &orb = cqspt::updateOrbit(universe.get<cqspt::Orbit>(body));
        //cqspt::updatePos(universe.get<cqspt::Kinematics>(body), orb);
    }
}

int conquerspace::common::systems::SysOrbit::Interval() { return 1; }

void conquerspace::common::systems::SysPath::DoSystem(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;

    auto bodies = universe.view<cqspt::MoveTarget, cqspt::Position>(entt::exclude<cqspt::Orbit>);
    for (entt::entity body : bodies) {
        cqspt::Position& bodykin = universe.get<cqspt::Position>(body);
        cqspt::Position targetkin = cqspt::toVec2(universe.get<cqspt::Orbit>(universe.get<cqspt::MoveTarget>(body).target));
        cqspt::Vec2 path = targetkin - bodykin;
        float velocity = .5f;
        if (targetkin.distance(bodykin) < velocity) {
            bodykin = targetkin;
        } else {
           bodykin += (velocity * path.normalize());
        }
    }
}

int conquerspace::common::systems::SysPath::Interval() { return 1; }
