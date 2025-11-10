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
#include "common/loading/loadcities.h"

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "common/actions/factoryconstructaction.h"
#include "common/components/area.h"
#include "common/components/coordinates.h"
#include "common/components/infrastructure.h"
#include "common/components/market.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/spaceport.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"

namespace cqsp::common::loading {
namespace {
struct ConnectedCities {
    // Holder class for the names of all the cities connected to this particular city
    std::vector<std::string> entities;
};
}  // namespace

bool CityLoader::LoadValue(const Hjson::Value& values, Node& node) {
    //SPDLOG_INFO("Load the city);
    std::string planet = values["planet"].to_string();
    double longi = values["coordinates"]["longitude"].to_double();
    double lat = values["coordinates"]["latitude"].to_double();
    auto& sc = node.emplace<components::types::SurfaceCoordinate>(lat, longi);
    Node planet_node(node, universe.planets[planet]);
    sc.planet = planet_node;

    planet_node.get_or_emplace<components::Habitation>().settlements.push_back(node);
    //SPDLOG_INFO("Load Timezone");
    if (!values["timezone"].empty()) {
        entt::entity tz = universe.time_zones[values["timezone"].to_string()];
        node.emplace<components::CityTimeZone>(tz);
    }

    //SPDLOG_INFO("Load Population");
    auto& settlement = node.emplace<components::Settlement>();
    std::string identifier = node.get<components::Identifier>().identifier;
    // Load population
    if (!values["population"].empty()) {
        Hjson::Value population = values["population"];
        for (int i = 0; i < population.size(); i++) {
            Hjson::Value population_seg = population[i];
            Node pop_node(universe);

            auto size = population_seg["size"].to_int64();
            double standard_of_living = 0;
            if (!population_seg["sol"].empty()) {
                standard_of_living = population_seg["sol"].to_double();
            }

            double balance = 0;
            if (!population_seg["balance"].empty()) {
                balance = population_seg["balance"].to_double();
            }

            int64_t labor_force = size / 2;
            if (!population_seg["labor_force"].empty()) {
                labor_force = population_seg["labor_force"].to_int64();
            }

            auto& segment = pop_node.emplace<components::PopulationSegment>();
            segment.population = size;
            segment.labor_force = labor_force;
            segment.standard_of_living = standard_of_living;
            pop_node.emplace<components::LaborInformation>();
            auto& wallet = pop_node.emplace<components::Wallet>();
            wallet = balance;
            settlement.population.push_back(pop_node);
        }
    } else {
        Node pop_node(universe);

        auto size = 50000;
        int64_t labor_force = size / 2;

        auto& segment = pop_node.emplace<components::PopulationSegment>();
        segment.population = size;
        segment.labor_force = labor_force;
        pop_node.emplace<components::LaborInformation>();
        settlement.population.push_back(pop_node);
        SPDLOG_WARN("City {} does not have any population", identifier);
    }
    //SPDLOG_INFO("Load Industry");
    node.emplace<components::ResourceLedger>();

    // Industry and economy
    auto& industry = node.emplace<components::IndustrialZone>();
    auto& market = node.emplace<components::Market>();
    market.parent_market = universe.planets[planet];
    // Get the connected markets
    if (!values["connections"].empty() && values["connections"].type() == Hjson::Type::Vector) {
        // Get connected cities and then see if they're done
        Hjson::Value connected = values["connections"];
        if (!connected.empty()) {
            auto& conn = node.emplace<ConnectedCities>();
            for (int i = 0; i < connected.size(); i++) {
                Hjson::Value& val = connected[i];
                conn.entities.push_back(val.to_string());
            }
        }
    }
    // Commercial area
    Node commercial_node(universe);

    commercial_node.emplace<components::Employer>();
    commercial_node.emplace<components::Commercial>(node, 0);

    industry.industries.push_back(commercial_node);

    if (!values["industry"].empty()) {
        Hjson::Value industry_hjson = values["industry"];

        for (int i = 0; i < industry_hjson.size(); i++) {
            Hjson::Value ind_val = industry_hjson[i];
            auto recipe = ind_val["recipe"].to_string();
            auto productivity = ind_val["productivity"].to_double();
            if (universe.recipes.find(recipe) == universe.recipes.end()) {
                SPDLOG_INFO("Recipe {} not found in city {}", recipe, identifier);
                continue;
            }
            double wage = 10;
            if (!ind_val["wage"].empty()) {
                wage = ind_val["wage"].to_double();
            }
            Node rec_ent(universe, universe.recipes[recipe]);

            actions::CreateFactory(node, rec_ent, productivity);
        }
    }
    //SPDLOG_INFO("Load SpacePort");
    if (!values["space-port"].empty()) {
        // Add space port
        auto& space_port = node.emplace<components::infrastructure::SpacePort>();
        space_port.reference_body = sc.planet;
    }
    //SPDLOG_INFO("Load Country");
    if (!values["country"].empty()) {
        if (universe.countries.find(values["country"]) != universe.countries.end()) {
            Node country_node(universe, universe.countries[values["country"]]);
            node.emplace<components::Governed>(country_node);
            // Add self to country?
            country_node.get_or_emplace<components::CountryCityList>().city_list.push_back(node);

        } else {
            SPDLOG_INFO("City {} has country {}, but it's undefined", identifier, values["country"].to_string());
        }
    } else {
        SPDLOG_WARN("City {} has no country", identifier);
    }
    //SPDLOG_INFO("Load Provinces");
    if (!values["province"].empty()) {
        if (universe.provinces[values["province"]] != universe.provinces.end()) {
            Node province_node(universe, universe.provinces[values["province"]]);
            // Now add self to province
            province_node.get<components::Province>().cities.push_back(node);
        } else {
            SPDLOG_WARN("City {} has province {}, but it's undefined", identifier, values["province"].to_string());
        }
    }

    //SPDLOG_INFO("Add infrastructure to city");
    auto& infrastructure = node.emplace<components::infrastructure::CityInfrastructure>();
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
            auto& highway = node.emplace<components::infrastructure::Highway>();
            highway.extent = values["infrastructure"]["highway"].to_double();
        }
    }

    //SPDLOG_INFO("Load Tags");
    if (!values["tags"].empty()) {
        for (int i = 0; i < values["tags"].size(); i++) {
            if (values["tags"][i].to_string() == "capital") {
                // Then it's a capital city of whatever country it's in
                node.emplace<components::CapitalCity>();
                // Add to parent country
                if (node.any_of<components::Governed>()) {
                    Node governor_node(universe, node.get<components::Governed>().governor);
                    auto& country_comp = universe.get<components::Country>(governor_node);
                    if (country_comp.capital_city != entt::null) {
                        // Get name
                        SPDLOG_INFO("Country {} already has a capital; {} will be replaced with {}",
                                    util::GetName(universe, governor_node),
                                    util::GetName(universe, country_comp.capital_city), util::GetName(universe, node));
                        // Remove capital tag on the other capital city
                        Node(universe, country_comp.capital_city).remove<components::CapitalCity>();
                    }
                    country_comp.capital_city = node;
                }
            }
        }
    }
    //SPDLOG_INFO("Save City");
    universe.cities[identifier] = node;
    return true;
}

/**
 * Loads the city and sets the parent.
 */
void CityLoader::PostLoad(const Node& node) {
    if (node.all_of<ConnectedCities>()) {
        auto& connected = node.get<ConnectedCities>();
        auto& market = node.get<components::Market>();
        for (auto& entity : connected.entities) {
            market.connected_markets.emplace(universe.cities[entity]);
        }
        node.remove<ConnectedCities>();
    }
}
}  // namespace cqsp::common::loading
