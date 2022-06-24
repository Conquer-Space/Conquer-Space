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

#include <spdlog/spdlog.h>

#include <string>

#include "common/components/coordinates.h"
#include "common/components/surface.h"
#include "common/components/population.h"
#include "common/components/economy.h"
#include "common/components/area.h"
#include "common/systems/actions/factoryconstructaction.h"
#include "common/components/name.h"
#include "common/components/infrastructure.h"

namespace cqsp::common::systems::loading {
bool CityLoader::LoadValue(
    const Hjson::Value& values, entt::entity entity) {
    // Load the city
    std::string planet = values["planet"].to_string();
    double longi = values["coordinates"]["longitude"].to_double();
    double lat = values["coordinates"]["latitude"].to_double();
    universe.emplace<components::types::SurfaceCoordinate>(entity, lat, longi);
    universe.get_or_emplace<components::Habitation>(universe.planets[planet])
        .settlements.push_back(entity);

    if (!values["timezone"].empty()) {
        entt::entity tz = universe.time_zones[values["timezone"].to_string()];
        universe.emplace<components::CityTimeZone>(entity, tz);
    }

    auto& settlement = universe.emplace<components::Settlement>(entity);
    // Load population
    if (!values["population"].empty()) {
        Hjson::Value population = values["population"];
        for (int i = 0; i < population.size(); i++) {
            Hjson::Value population_seg = population[i];
            entt::entity pop_ent = universe.create();

            auto size = population_seg["size"].to_int64();
            universe.emplace<components::PopulationSegment>(pop_ent).population = size;
            universe.emplace<components::Employee>(pop_ent);
            settlement.population.push_back(pop_ent);
        }
    } else {
        SPDLOG_WARN("City {} does not have any population",
                    universe.get<components::Identifier>(entity).identifier);
    }

    universe.emplace<components::ResourceLedger>(entity);

    // Industry and economy
    auto& industry = universe.emplace<components::Industry>(entity);

    // Commercial area
    entt::entity commercial = universe.create();

    universe.emplace<components::Employer>(commercial);
    universe.emplace<components::Commercial>(commercial, entity, 0);

    industry.industries.push_back(commercial);

    if (!values["industry"].empty()) {
        Hjson::Value industry_hjson = values["industry"];
        for (int i = 0; i < industry_hjson.size(); i++) {
            Hjson::Value ind_val = industry_hjson[i];
            auto recipe = ind_val["recipe"].to_string();
            auto productivity = ind_val["productivity"].to_double();
            entt::entity rec_ent = universe.recipes[recipe];

            actions::CreateFactory(universe, entity, rec_ent, productivity);
        }
    }

    if (!values["space-port"].empty()) {
        // Add space port
        universe.emplace<components::infrastructure::SpacePort>(entity);
    }
    return true;
}

void CityLoader::PostLoad(const entt::entity& entity) {}
}  // namespace cqsp::common::systems::loading
