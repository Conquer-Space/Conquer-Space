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
#include "core/loading/loadcities.h"

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "core/actions/factoryconstructaction.h"
#include "core/components/area.h"
#include "core/components/coordinates.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"

namespace cqsp::core::loading {
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

    planet_node.get_or_emplace<components::Settlements>().settlements.push_back(node);
    //SPDLOG_INFO("Load Timezone");
    if (!values["timezone"].empty()) {
        entt::entity tz = universe.time_zones[values["timezone"].to_string()];
        node.emplace<components::CityTimeZone>(tz);
    }
    std::string identifier = node.get<components::Identifier>().identifier;

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
        if (universe.provinces.contains(values["province"])) {
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

void CityLoader::ParseIndustry(const Hjson::Value& industry_hjson, Node& node, std::string_view identifier) {
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

void CityLoader::PostLoad(const Node& node) {}
}  // namespace cqsp::core::loading
