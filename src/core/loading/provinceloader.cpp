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
#include "core/loading/provinceloader.h"

#include <spdlog/spdlog.h>

#include "core/actions/factoryconstructaction.h"
#include "core/components/infrastructure.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"

namespace cqsp::core::loading {
bool ProvinceLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Get country
    std::string country_identifier = values["country"].to_string();
    std::string planet_identifier = values["planet"].to_string();
    Node planet_node(universe, universe.planets[planet_identifier]);
    const auto& identifier = node.get<components::Identifier>().identifier;

    Node country_node(universe, universe.countries[country_identifier]);
    node.emplace<components::Province>(country_node);
    auto& color = node.emplace<components::ProvinceColor>(std::stoi(values["color"][0]), std::stoi(values["color"][1]),
                                                          std::stoi(values["color"][2]));
    if (universe.provinces.find(identifier) == universe.provinces.end()) {
        universe.provinces[identifier] = node;
    } else {
        SPDLOG_WARN("Province {} conflicts with an already preexisting province", identifier);
    }
    // Add province to country
    // check if it is assigned to a country
    country_node.get_or_emplace<components::CountryCityList>().province_list.push_back(node);
    universe.province_colors[planet_node][static_cast<int>(color)] = node;
    universe.colors_province[planet_node][node] = static_cast<int>(color);

    planet_node.get_or_emplace<components::Settlements>().settlements.push_back(node);
    //SPDLOG_INFO("Load Timezone");
    if (!values["timezone"].empty()) {
        entt::entity tz = universe.time_zones[values["timezone"].to_string()];
        node.emplace<components::CityTimeZone>(tz);
    }

    //SPDLOG_INFO("Load Population");
    auto& settlement = node.emplace<components::Settlement>();

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
    market.parent_market = planet_node;

    // Commercial area
    Node commercial_node(universe);

    commercial_node.emplace<components::Employer>();
    commercial_node.emplace<components::Commercial>(node, 0);

    industry.industries.push_back(commercial_node);

    if (!values["industry"].empty()) {
        Hjson::Value industry_hjson = values["industry"];

        ParseIndustry(industry_hjson, node, identifier);
    }
    //SPDLOG_INFO("Load SpacePort");
    if (!values["space-port"].empty()) {
        // Add space port
        auto& space_port = node.emplace<components::infrastructure::SpacePort>();
        space_port.reference_body = planet_node;
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
                    country_comp.capital_city = node;
                    if (country_comp.capital_city == entt::null) {
                        continue;
                    }
                    // Get name
                    SPDLOG_INFO("Country {} already has a capital; {} will be replaced with {}",
                                util::GetName(universe, governor_node),
                                util::GetName(universe, country_comp.capital_city), util::GetName(universe, node));
                    // Remove capital tag on the other capital city
                    Node(universe, country_comp.capital_city).remove<components::CapitalCity>();
                }
            }
        }
    }
    return true;
}

void ProvinceLoader::PostLoad(const Node& node) {}

void ProvinceLoader::ParseIndustry(const Hjson::Value& industry_hjson, Node& node, std::string_view identifier) {
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
}  // namespace cqsp::core::loading
