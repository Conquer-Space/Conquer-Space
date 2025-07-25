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
#include "common/scripting/luafunctions.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/economy.h"
#include "common/components/event.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/science.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/scripting/functionreg.h"
#include "common/systems/actions/cityactions.h"
#include "common/systems/actions/factoryconstructaction.h"
#include "common/systems/actions/shiplaunchaction.h"
#include "common/systems/economy/markethelpers.h"
#include "common/systems/science/labs.h"
#include "common/systems/science/technology.h"
#include "common/util/nameutil.h"
#include "common/util/random/stdrandom.h"
#include "common/util/utilnumberdisplay.h"

namespace cqspb = cqsp::common::components::bodies;
namespace cqsps = cqsp::common::components::ships;
namespace cqspt = cqsp::common::components::types;
namespace cqspc = cqsp::common::components;

/**
 * Notes:
 * If you want to return arrays you need to encapsulate it on sol::as_table
 */
namespace {
/// <summary>
/// Initializes functions for RNG
/// </summary>
/// <param name="app"></param>
void FunctionRandom(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    // RNG
    // Make namespace
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("random", [&](int low, int high) { return universe.random->GetRandomInt(low, high); });

    REGISTER_FUNCTION("random_normal_int",
                      [&](int mean, int sd) { return universe.random->GetRandomNormalInt(mean, sd); });
}

void FunctionUniverseBodyGen(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("add_planet", [&]() {
        entt::entity planet = universe.create();
        auto& body = universe.emplace<cqspb::Body>(planet);
        universe.emplace<cqspb::Planet>(planet);
        return planet;
    });

    REGISTER_FUNCTION("add_star", [&]() {
        entt::entity star = universe.create();
        universe.emplace<cqspb::Star>(star);
        auto& body = universe.emplace<cqspb::Body>(star);
        universe.emplace<cqspb::LightEmitter>(star);
        return star;
    });

    REGISTER_FUNCTION("set_orbit", [&](entt::entity orbital_entity, double semi_major_axis, double eccentricity,
                                       double inclination, double LAN, double w, double M0) {
        cqspt::Orbit& orb = universe.emplace<cqspt::Orbit>(orbital_entity);
        orb.eccentricity = eccentricity;
        orb.semi_major_axis = semi_major_axis;
        orb.inclination = inclination;
        orb.LAN = LAN;
        orb.w = w;
        orb.M0 = M0;
        auto& kinematics = universe.emplace<cqspt::Kinematics>(orbital_entity);
        cqspt::UpdatePos(kinematics, orb);
    });

    REGISTER_FUNCTION("set_system_position", [&](entt::entity orbital_ent, double x, double y) {
        auto& coord = universe.get_or_emplace<cqspt::GalacticCoordinate>(orbital_ent);
        coord.x = x;
        coord.y = y;
    });

    REGISTER_FUNCTION("set_radius", [&](entt::entity body, int radius) {
        cqspb::Body& bod = universe.get<cqspb::Body>(body);
        bod.radius = radius;
    });

    REGISTER_FUNCTION("create_terrain", [&](entt::entity planet, int seed, entt::entity terrain_type) {
        static_cast<void>(universe.emplace<cqspb::Terrain>(planet, seed, terrain_type));
    });
}

void FunctionCivilizationGen(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("add_civilization", [&]() {
        entt::entity civ = universe.create();
        universe.emplace<cqspc::Organization>(civ);
        return civ;
    });

    REGISTER_FUNCTION("set_owner", [&](entt::entity entity, entt::entity owner) {
        auto& gov = universe.get_or_emplace<cqspc::Governed>(entity);
        gov.governor = owner;
    });

    REGISTER_FUNCTION("get_governed", [&](entt::entity governor) {
        auto view = universe.view<cqspc::Governed>();
        // this is probably an antiBpattern but ah well
        std::vector<entt::entity> governed;
        for (auto entity : view) {
            if (universe.get<cqspc::Governed>(entity).governor == governor) {
                governed.push_back(entity);
            }
        }
        return sol::as_table(governed);
    });

    REGISTER_FUNCTION("get_owned_cities", [&](entt::entity player) {
        return sol::as_table(universe.get<cqspc::CountryCityList>(player).province_list);
    });

    REGISTER_FUNCTION("is_player",
                      [&](entt::entity civ) { return static_cast<bool>(universe.all_of<cqspc::Player>(civ)); });

    REGISTER_FUNCTION("add_planet_habitation",
                      [&](entt::entity planet) { universe.emplace<cqspc::Habitation>(planet); });

    REGISTER_FUNCTION("add_planet_settlement", [&](entt::entity planet, double lat, double longi) {
        return cqsp::common::actions::CreateCity(universe, planet, lat, longi);
    });
}

void FunctionEconomy(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    namespace cqspa = cqsp::common::systems::actions;
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("create_industries", [&](entt::entity city) { universe.emplace<cqspc::IndustrialZone>(city); });

    REGISTER_FUNCTION("add_industry", [&](entt::entity city, entt::entity entity) {
        universe.get<cqspc::IndustrialZone>(city).industries.push_back(entity);
    });

    REGISTER_FUNCTION("create_factory", [&](entt::entity city, entt::entity recipe, float productivity) {
        entt::entity factory = cqspa::CreateFactory(universe, city, recipe, productivity);
        return factory;
    });

    REGISTER_FUNCTION("add_production", [&](entt::entity factory) {
        // Factory will produce in the first tick
        universe.emplace<cqspc::FactoryProducing>(factory);
    });

    REGISTER_FUNCTION("set_power_consumption", [&](entt::entity factory, double max, double min) {
        universe.emplace<cqspc::infrastructure::PowerConsumption>(factory, max, min, 0.f);
        return factory;
    });

    REGISTER_FUNCTION("add_power_plant", [&](entt::entity city, double productivity) {
        entt::entity entity = universe.create();
        universe.emplace<cqspc::infrastructure::PowerPlant>(entity, productivity);
        universe.get<cqspc::IndustrialZone>(city).industries.push_back(entity);
        return entity;
    });

    REGISTER_FUNCTION("create_commercial_area",
                      [&](entt::entity city) { return cqspa::CreateCommercialArea(universe, city); });

    REGISTER_FUNCTION("set_resource_consume", [&](entt::entity entity, entt::entity good, double amount) {
        auto& consumption = universe.get_or_emplace<cqspc::ResourceConsumption>(entity);
        consumption[good] = amount;
    });

    REGISTER_FUNCTION("set_resource", [&](entt::entity planet, entt::entity resource, int seed) {
        auto& dist = universe.get_or_emplace<cqspc::ResourceDistribution>(planet);
        dist.dist[resource] = seed;
    });

    // TODO(EhWhoAmI): Will have to fix the documentation for this so that it looks neater
    // The macro cannot take lambadas that contain templates that contain commas
    auto lambda = [&]() {
        /*entt::entity entity = universe.create();
        auto& market = universe.emplace<cqspc::Market>(entity);
        universe.emplace<cqspc::ResourceStockpile>(entity);
        // Set the market prices
        // TODO(EhWhoAmI): This is a bandaid solution, please fix this

        auto view = universe.view<cqspc::Good, cqspc::Price>();
        for (entt::entity entity : view) {
            // Assign price to market
            market.prices[entity] = universe.get<cqspc::Price>(entity);
        }*/
        entt::entity market_entity = cqsp::common::systems::economy::CreateMarket(universe);
        // Set prices of market
        auto view = universe.view<cqspc::Good, cqspc::Price>();
        auto& market = universe.get<cqsp::common::components::Market>(market_entity);
        for (entt::entity entity : view) {
            // Assign price to market
            market.market_information[entity].price = universe.get<cqspc::Price>(entity);
        }
        return market_entity;
        // return entity;
    };
    REGISTER_FUNCTION("create_market", lambda);

    REGISTER_FUNCTION("place_market", [&](entt::entity market, entt::entity planet) {
        universe.emplace<cqspc::MarketCenter>(planet, market);
    });

    REGISTER_FUNCTION("attach_market", [&](entt::entity market_entity, entt::entity participant) {
        cqsp::common::systems::economy::AddParticipant(universe, market_entity, participant);
    });

    REGISTER_FUNCTION("get_balance", [&](entt::entity participant) {
        return universe.get_or_emplace<cqspc::Wallet>(participant).GetBalance();
    });

    REGISTER_FUNCTION("add_balance", [&](entt::entity participant, double balance) {
        universe.get_or_emplace<cqspc::Wallet>(participant) += balance;
    });

    auto get_planetary_markets = [&]() {
        auto view = universe.view<cqspc::Market, cqspc::PlanetaryMarket>();
        std::vector<entt::entity> markets;
        for (entt::entity entity : view) {
            markets.push_back(entity);
        }
        return sol::as_table(markets);
    };
    REGISTER_FUNCTION("get_planetary_markets", get_planetary_markets);
}

void FunctionUser(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("set_name", [&](entt::entity entity, const std::string& name) {
        universe.emplace_or_replace<cqspc::Name>(entity, name);
    });

    REGISTER_FUNCTION("to_human_string", [&](int64_t number) { return cqsp::util::LongToHumanString(number); });

    REGISTER_FUNCTION("get_name", [&](entt::entity entity) { return cqsp::common::util::GetName(universe, entity); });

    REGISTER_FUNCTION("get_random_name", [&](const std::string& name_gen, const std::string& rule) {
        return universe.name_generators[name_gen].Generate(rule);
    });
}

void FunctionPopulation(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("add_population_segment", [&](entt::entity settlement, uint64_t popsize) {
        entt::entity population = universe.create();
        universe.emplace<cqspc::PopulationSegment>(population, popsize);
        universe.emplace<cqspc::ResourceStockpile>(population);
        universe.emplace<cqspc::LaborInformation>(population);
        // Add to planet list
        universe.get<cqspc::Settlement>(settlement).population.push_back(population);

        return population;
    });

    REGISTER_FUNCTION("get_segment_size",
                      [&](entt::entity segment) { return universe.get<cqspc::PopulationSegment>(segment).population; });

    // Get population segments of a planet
    REGISTER_FUNCTION("get_segments",
                      [&](entt::entity planet) { return universe.get<cqspc::Settlement>(planet).population; });

    // Get cities of a planet
    REGISTER_FUNCTION("get_cities",
                      [&](entt::entity planet) { return universe.get<cqspc::Habitation>(planet).settlements; });

    REGISTER_FUNCTION("get_city", [&](const std::string& planet) { return universe.cities[planet]; });
}

void FunctionShips(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("create_ship", [&](entt::entity civ, entt::entity orbit, entt::entity starsystem) {
        return cqsp::common::systems::actions::CreateShip(universe, civ, orbit, starsystem);
    });
}

void FunctionEvent(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("push_event", [&](entt::entity entity, sol::table event_table) {
        auto& queue = universe.get_or_emplace<cqsp::common::event::EventQueue>(entity);
        auto event = std::make_shared<cqsp::common::event::Event>();
        event->title = event_table["title"];
        SPDLOG_INFO("Parsing event \"{}\"", event->title);
        event->content = event_table["content"];
        event->image = event_table["image"];
        sol::optional<std::vector<sol::table>> optional = event_table["actions"];
        if (optional) {
            for (auto& action : *optional) {
                if (action == sol::nil) {
                    continue;
                }
                auto event_result = std::make_shared<cqsp::common::event::EventResult>();
                event_result->name = action["name"];
                sol::optional<std::string> tooltip = action["tooltip"];
                if (tooltip) {
                    event_result->tooltip = *tooltip;
                }

                event->table = event_table;
                sol::optional<sol::function> f = action["action"];
                event_result->has_event = f.has_value();
                if (f) {
                    event_result->action = *f;
                }
                event->actions.push_back(event_result);
            }
        }
        queue.events.push_back(event);
    });
}

void FunctionResource(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("add_resource", [&](entt::entity storage, entt::entity resource, int amount) {
        // Add resources to the resource stockpile
        universe.get<cqspc::ResourceStockpile>(storage)[resource] += amount;
    });

    REGISTER_FUNCTION("get_resource_count", [&](entt::entity stockpile, entt::entity resource) {
        return universe.get<cqspc::ResourceStockpile>(stockpile)[resource];
    });
}

void FunctionCivilizations(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("get_player", [&]() { return universe.view<cqspc::Player>().front(); });
    REGISTER_FUNCTION("get_capital_city",
                      [&](entt::entity civ) { return universe.get<cqspc::Country>(civ).capital_city; });
}

void FunctionScience(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("create_lab", [&]() { return cqsp::common::systems::science::CreateLab(universe); });

    REGISTER_FUNCTION("add_science", [&](entt::entity lab, entt::entity research, double progress) {
        cqsp::common::systems::science::AddScienceResearch(universe, lab, research, progress);
    });

    REGISTER_FUNCTION("add_tech_progress", [&](entt::entity entity) {
        universe.emplace<cqsp::common::components::science::TechnologicalProgress>(entity);
        universe.emplace<cqsp::common::components::science::ScientificResearch>(entity);
    });

    REGISTER_FUNCTION("complete_technology", [&](entt::entity entity, entt::entity tech) {
        cqsp::common::systems::science::ResearchTech(universe, entity, tech);
    });

    REGISTER_FUNCTION("research_technology", [&](entt::entity entity, entt::entity tech) {
        auto& res = universe.get<cqsp::common::components::science::ScientificResearch>(entity);
        res.current_research[tech] = 0;
    });

    REGISTER_FUNCTION("add_potential_tech", [&](entt::entity entity, entt::entity tech) {
        auto& res = universe.get<cqsp::common::components::science::ScientificResearch>(entity);
        res.potential_research.insert(tech);
    });
}

// this is just meant for debugging and is not performant at all
sol::table GetMarketTable(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine,
                          entt::entity market) {
    sol::table market_table = script_engine.create_table_with();

    cqsp::common::components::Market& market_component = universe.get<cqsp::common::components::Market>(market);
    auto goods_view = universe.view<cqsp::common::components::Price>();
    for (entt::entity good : goods_view) {
        sol::table good_table = script_engine.create_table_with();
        good_table["price"] = market_component.price[good];
        good_table["supply"] = market_component.supply()[good];
        good_table["demand"] = market_component.demand()[good];
        good_table["sd_ratio"] = market_component.sd_ratio[good];
        good_table["consumption"] = market_component.consumption[good];
        good_table["production"] = market_component.production[good];
        good_table["trade"] = market_component.trade[good];
        market_table.set(good, good_table);
        // Now get all the values for goods
    }
    return market_table;
}

void FunctionTrade(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(core);

    REGISTER_FUNCTION("get_market_table",
                      [&](entt::entity market) { return GetMarketTable(universe, script_engine, market); });
}
}  // namespace

void cqsp::scripting::LoadFunctions(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    FunctionCivilizationGen(universe, script_engine);
    FunctionCivilizations(universe, script_engine);
    FunctionEconomy(universe, script_engine);
    FunctionPopulation(universe, script_engine);
    FunctionRandom(universe, script_engine);
    FunctionUniverseBodyGen(universe, script_engine);
    FunctionUser(universe, script_engine);
    FunctionEvent(universe, script_engine);
    FunctionShips(universe, script_engine);
    FunctionResource(universe, script_engine);
    FunctionScience(universe, script_engine);
    FunctionTrade(universe, script_engine);
}
