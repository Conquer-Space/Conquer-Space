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
#include "common/systems/actions/cityactions.h"

#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/surface.h"

namespace components = cqsp::common::components;
namespace types = components::types;
using types::SurfaceCoordinate;
using components::Habitation;
using components::Settlement;

using entt::entity;

entity cqsp::common::systems::actions::CreateCity(Universe& universe, entity planet, double lat, double longi) {
    entity settlement = universe.create();
    universe.emplace<Settlement>(settlement);
    universe.emplace<SurfaceCoordinate>(settlement, lat, longi);

    // Add to planet list

    if (universe.all_of<Habitation>(planet)) {
        universe.get<Habitation>(planet).settlements.push_back(settlement);
    } else {
        universe.emplace<Habitation>(planet);
        universe.get<Habitation>(planet).settlements.push_back(settlement);
    }

    return settlement;
}
