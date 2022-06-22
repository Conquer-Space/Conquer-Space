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
#include "common/systems/loading/loadcities.h"

#include <string>

#include "common/components/coordinates.h"
#include "common/components/surface.h"

namespace cqsp::common::systems::loading {
bool CityLoader::LoadValue(
    const Hjson::Value& values, entt::entity entity) {
    // Load the city
    std::string planet = values["planet"].to_string();
    double longi = values["coordinates"]["longitude"].to_double();
    double lat = values["coordinates"]["latitude"].to_double();
    universe.emplace<components::types::SurfaceCoordinate>(entity, lat, longi);
    universe.emplace<components::Settlement>(entity);
    universe.get_or_emplace<components::Habitation>(universe.planets[planet])
        .settlements.push_back(entity);

    if (!values["timezone"].empty()) {
        entt::entity tz = universe.time_zones[values["timezone"].to_string()];
        universe.emplace<components::CityTimeZone>(entity, tz);
    }
    // Add to city
    return true;
}

void CityLoader::PostLoad(const entt::entity& entity) {}
}  // namespace cqsp::common::systems::loading
