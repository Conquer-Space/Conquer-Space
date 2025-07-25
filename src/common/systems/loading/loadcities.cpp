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
#include "common/systems/loading/loadcities.h"

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "common/components/area.h"
#include "common/components/coordinates.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/spaceport.h"
#include "common/components/surface.h"
#include "common/systems/actions/factoryconstructaction.h"
#include "common/util/nameutil.h"

namespace cqsp::common::systems::loading {
namespace {
struct ConnectedCities {
    // Holder class for the names of all the cities connected to this particular city
    std::vector<std::string> entities;
};
}  // namespace
bool CityLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    // Load the city
    std::string planet = values["planet"].to_string();
    double longi = values["coordinates"]["longitude"].to_double();
    double lat = values["coordinates"]["latitude"].to_double();
    auto& sc = universe.emplace<components::types::SurfaceCoordinate>(entity, lat, longi);
    sc.planet = universe.planets[planet];

    universe.get_or_emplace<components::Habitation>(universe.planets[planet]).settlements.push_back(entity);

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
            int64_t labor_force = size / 2;
            if (!population_seg["labor_force"].empty()) {
                labor_force = population_seg["labor_force"].to_int64();
            }

            auto& segment = universe.emplace<components::PopulationSegment>(pop_ent);
            segment.population = size;
            segment.labor_force = labor_force;
            universe.emplace<components::LaborInformation>(pop_ent);
            settlement.population.push_back(pop_ent);
        }
    } else {
        entt::entity pop_ent = universe.create();

        auto size = 50000;
        int64_t labor_force = size / 2;

        auto& segment = universe.emplace<components::PopulationSegment>(pop_ent);
        segment.population = size;
        segment.labor_force = labor_force;
        universe.emplace<components::LaborInformation>(pop_ent);
        settlement.population.push_back(pop_ent);
        SPDLOG_WARN("City {} does not have any population", universe.get<components::Identifier>(entity).identifier);
    }

    universe.emplace<components::ResourceLedger>(entity);

    // Industry and economy
    auto& industry = universe.emplace<components::IndustrialZone>(entity);
    auto& market = universe.emplace<components::Market>(entity);
    // Get the connected markets
    if (!values["connections"].empty() && values["connections"].type() == Hjson::Type::Vector) {
        // Get connected cities and then see if they're done
        Hjson::Value connected = values["connections"];
        if (!connected.empty()) {
            auto& conn = universe.emplace<ConnectedCities>(entity);
            for (int i = 0; i < connected.size(); i++) {
                Hjson::Value& val = connected[i];
                conn.entities.push_back(val.to_string());
            }
        }
    }
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
            if (universe.recipes.find(recipe) == universe.recipes.end()) {
                SPDLOG_INFO("Recipe {} not found in city {}", recipe,
                            universe.get<components::Identifier>(entity).identifier);
                continue;
            }
            entt::entity rec_ent = universe.recipes[recipe];

            actions::CreateFactory(universe, entity, rec_ent, productivity);
        }
    }

    if (!values["space-port"].empty()) {
        // Add space port
        universe.emplace<components::infrastructure::SpacePort>(entity);
    }

    if (!values["country"].empty()) {
        if (universe.countries.find(values["country"]) != universe.countries.end()) {
            entt::entity country = universe.countries[values["country"]];
            universe.emplace<components::Governed>(entity, country);
            // Add self to country?
            universe.get_or_emplace<components::CountryCityList>(country).city_list.push_back(entity);
        } else {
            SPDLOG_INFO("City {} has country {}, but it's undefined",
                        universe.get<components::Identifier>(entity).identifier, values["country"].to_string());
        }
    } else {
        SPDLOG_WARN("City {} has no country", universe.get<components::Identifier>(entity).identifier);
    }

    if (!values["province"].empty()) {
        if (universe.provinces[values["province"]] != universe.provinces.end()) {
            entt::entity province = universe.provinces[values["province"]];
            // Now add self to province
            universe.get<components::Province>(province).cities.push_back(entity);
        } else {
            SPDLOG_WARN("City {} has province {}, but it's undefined",
                        universe.get<components::Identifier>(entity).identifier, values["province"].to_string());
        }
    }

    // Add infrastructure to city
    auto& infrastructure = universe.emplace<components::infrastructure::CityInfrastructure>(entity);
    if (!values["transport"].empty()) {
        infrastructure.default_purchase_cost = values["transport"].to_double();
    } else {
        infrastructure.default_purchase_cost = 100;
    }
    if (!values["infrastructure"].empty()) {
        SPDLOG_INFO("Has Infrastructure");
        // Load infrastructure
        if (!values["infrastructure"]["highway"].empty()) {
            SPDLOG_INFO("Has highway");
            // Set the stuff
            auto& highway = universe.emplace<components::infrastructure::Highway>(entity);
            highway.extent = values["infrastructure"]["highway"].to_double();
        }
    }

    if (!values["tags"].empty()) {
        for (int i = 0; i < values["tags"].size(); i++) {
            if (values["tags"][i].to_string() == "capital") {
                // Then it's a capital city of whatever country it's in
                universe.emplace<components::CapitalCity>(entity);
                // Add to parent country
                if (universe.any_of<components::Governed>(entity)) {
                    entt::entity governor = universe.get<components::Governed>(entity).governor;
                    auto& country_comp = universe.get<components::Country>(governor);
                    if (country_comp.capital_city != entt::null) {
                        // Get name
                        SPDLOG_INFO("Country {} already has a capital; {} will be replaced with {}",
                                    util::GetName(universe, governor),
                                    util::GetName(universe, country_comp.capital_city),
                                    util::GetName(universe, entity));
                        // Remove capital tag on the other capital city
                        universe.remove<components::CapitalCity>(country_comp.capital_city);
                    }
                    country_comp.capital_city = entity;
                }
            }
        }
    }
    universe.cities[universe.get<components::Identifier>(entity).identifier] = entity;
    return true;
}

void CityLoader::PostLoad(const entt::entity& entity) {
    if (universe.all_of<ConnectedCities>(entity)) {
        auto& connected = universe.get<ConnectedCities>(entity);
        auto& market = universe.get<components::Market>(entity);
        for (auto& entity : connected.entities) {
            market.connected_markets.emplace(universe.cities[entity]);
        }
        universe.remove<ConnectedCities>(entity);
    }
}
}  // namespace cqsp::common::systems::loading
