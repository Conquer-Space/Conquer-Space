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
#include "core/actions/cityactions.h"

#include "core/components/coordinates.h"
#include "core/components/name.h"
#include "core/components/surface.h"

namespace cqsp::core::actions {

Node CreateCity(Node& planet, components::types::SurfaceCoordinate coords) {
    Node city_node(planet.universe());
    city_node.emplace<components::Settlement>();
    city_node.emplace<components::types::SurfaceCoordinate>(coords);
    planet.get_or_emplace<components::Habitation>().settlements.push_back(city_node);
    return city_node;
}

Node CreateCity(Node& planet, double lat, double longi) {
    return CreateCity(planet, components::types::SurfaceCoordinate(lat, longi));
}

}  // namespace cqsp::core::actions