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
#include "common/systems/systransport.h"

#include "common/components/bodies.h"
#include "common/components/coordinates.h"

cqsp::common::components::types::kilometer
cqsp::common::systems::CalculatePlanetDistance(cqsp::common::Universe& universe,
                                               entt::entity planet,
                                               entt::entity city1,
                                               entt::entity city2) {
    namespace cqspc = cqsp::common::components;
    // Get planet radius
    auto& body = universe.get<cqspc::bodies::Body>(planet);
    auto& city1_pos = universe.get<cqspc::types::SurfaceCoordinate>(city1);
    auto& city2_pos = universe.get<cqspc::types::SurfaceCoordinate>(city2);
    return 0.0;
}
