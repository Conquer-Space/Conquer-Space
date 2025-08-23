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
#include "common/actions/cityactions.h"

#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/surface.h"

namespace cqsp::common::actions {

entt::entity CreateCity(Universe& universe, entt::entity planet, double lat, double longi) {
    entt::entity settlement = universe.create();
    universe.emplace<components::Settlement>(settlement);
    universe.emplace<components::types::SurfaceCoordinate>(settlement, lat, longi);

    // Add to planet list

    if (universe.all_of<components::Habitation>(planet)) {
        universe.get<components::Habitation>(planet).settlements.push_back(settlement);
    } else {
        universe.emplace<components::Habitation>(planet);
        universe.get<components::Habitation>(planet).settlements.push_back(settlement);
    }

    return settlement;
}
}  // namespace cqsp::common::actions