/*
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
#include "common/scripting/luafunctions.h"

#include <string>
#include <memory>
#include <vector>

#include "common/util/random/stdrandom.h"

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/surface.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/population.h"
#include "common/components/area.h"
#include "common/components/resource.h"
#include "common/components/ships.h"
#include "common/components/event.h"

#include "common/systems/actions/factoryconstructaction.h"
#include "common/systems/economy/markethelpers.h"
#include "common/systems/actions/shiplaunchaction.h"
#include "common/util/utilnumberdisplay.h"
#include "engine/asset.h"

// So that we can document in the future
#define REGISTER_FUNCTION(name, lambda) \
        script_engine.set_function(name, lambda)

void jsonToLuaObject(sol::table& table, std::string key, const Hjson::Value &j) {
    switch (j.type()) {
        case Hjson::Type::String:
            table[key] = j.to_string();
        case Hjson::Type::Bool:
            table[key] = j.operator bool();
        case Hjson::Type::Double:
            table[key] = j.to_double();
        case Hjson::Type::Int64:
            table[key] = j.to_int64();
        case Hjson::Type::Map: {
            sol::table obj;
            for (auto it = j.begin(); it != j.end(); ++it) {
                SPDLOG_INFO("{}", it->first);
                jsonToLuaObject(obj, it->first, it->second);
            }
            table[key] = obj;
        }
        case Hjson::Type::Vector: {

        }
        default:
        case Hjson::Type::Null:
            break;
    }
}
sol::object jsonToLuaObject(const Hjson::Value &j, const sol::state & lua) {
    switch (j.type()) {
        case Hjson::Type::String:
            return sol::make_object(lua, j.to_string());
        case Hjson::Type::Bool:
            return sol::make_object(lua, j.operator bool());
        case Hjson::Type::Double:
            return sol::make_object(lua, j.to_double());
        case Hjson::Type::Int64:
            return sol::make_object(lua, j.to_int64());
        case Hjson::Type::Map: {
            std::map<std::string, sol::object> obj;
            for (auto it = j.begin(); it != j.end(); ++it) {
                sol::object ob = jsonToLuaObject(it->second, lua);
                obj[it->first] = ob;
            }
            return sol::make_object(lua, obj);
        }
        case Hjson::Type::Vector: {
            std::vector<sol::object> vec;
            unsigned long i = 0;
            for (auto it = j.begin(); it != j.end(); ++it) {
                vec.push_back(jsonToLuaObject(it->second, lua));
            }
            return sol::make_object(lua, vec);
        }
        default:
        case Hjson::Type::Null:
            return sol::make_object(lua, sol::nil);
    }
}

void cqsp::scripting::LoadFunctions(cqsp::engine::Application& app) {
    cqsp::common::Universe& universe = app.GetUniverse();
    cqsp::scripting::ScriptInterface& script_engine = app.GetScriptInterface();

    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    namespace cqspc = cqsp::common::components;

    REGISTER_FUNCTION("require", [&](const char* script) {
        //app.GetAssetManager();
        // Get script from asset loader
        cqsp::asset::TextDirectoryAsset* asset = app.GetAssetManager().GetAsset<cqsp::asset::TextDirectoryAsset>("core:scripts");
        // Get the thing
        if (asset->paths.find(script) != asset->paths.end()) {
            return script_engine.require_script(script, asset->paths[script]);
        } else {
            SPDLOG_INFO("Cannot find require {}", script);
            return sol::make_object(script_engine, sol::nil);
        }
    });

    REGISTER_FUNCTION("get_text_asset", [&](const char* id) {
        cqsp::asset::TextAsset* asset =
            app.GetAssetManager().GetAsset<cqsp::asset::TextAsset>(id);
        return sol::make_object<std::string>(script_engine, asset->data);
    });

    /*
    * This does not work, for some reason, so it will be disabled for now
    REGISTER_
    FUNCTION("get_hjson_asset", [&](const char* string) -> sol::table {
        cqsp::asset::HjsonAsset* as = app.GetAssetManager().GetAsset<cqsp::asset::HjsonAsset>(string);
        // Create json object.
        sol::table tb = jsonToLuaObject(as->data, script_engine).as<sol::table>();
        spdlog::info("{}", tb.get_type());

        spdlog::info("{}", jsonToLuaObject(as->data, script_engine).get_type());
        return tb;
    });*/

    // Init civilization script
    REGISTER_FUNCTION("create_star_system", [&] () {
        entt::entity ent = universe.create();
        universe.emplace<cqspb::StarSystem>(ent);
        return ent;
     });

    // RNG
    REGISTER_FUNCTION("random", [&] (int low, int high) {
        return universe.random->GetRandomInt(low, high);
    });

    REGISTER_FUNCTION("random_normal_int", [&] (int mean, int sd) {
        return universe.random->GetRandomNormal(mean, sd);
    });

    REGISTER_FUNCTION("add_planet", [&] (entt::entity system) {
        entt::entity planet = universe.create();
        auto& body = universe.emplace<cqspb::Body>(planet);
        body.star_system = system;
        universe.emplace<cqspb::Planet>(planet);
        universe.get<cqspb::StarSystem>(system).bodies.push_back(planet);
        return planet;
    });

    REGISTER_FUNCTION("add_star", [&] (entt::entity system) {
        entt::entity star = universe.create();
        universe.emplace<cqspb::Star>(star);
        auto& body = universe.emplace<cqspb::Body>(star);
        body.star_system = system;

        universe.emplace<cqspb::LightEmitter>(star);

        universe.get<cqspb::StarSystem>(system).bodies.push_back(star);
        return star;
    });

    REGISTER_FUNCTION("set_orbit", [&] (entt::entity orbital_entity, double distance, double theta,
                                            double eccentricity, double argument) {
        cqspt::Orbit &orb = universe.emplace<cqspt::Orbit>(orbital_entity, theta, distance, eccentricity, argument, 40);
        auto& kinematics = universe.emplace<cqspt::Kinematics>(orbital_entity);
        cqspt::UpdatePos(kinematics, orb);
    });

    REGISTER_FUNCTION("set_radius", [&] (entt::entity body, int radius) {
        cqspb::Body &bod = universe.get<cqspb::Body>(body);
        bod.radius = radius;
    });

    REGISTER_FUNCTION("add_civilization", [&] () {
        entt::entity civ = universe.create();
        universe.emplace<cqspc::Organization>(civ);
        return civ;
    });

    REGISTER_FUNCTION("set_civilization_planet", [&] (entt::entity civ, entt::entity planet) {
        universe.get_or_emplace<cqspc::Civilization>(civ).starting_planet = planet;
    });

    REGISTER_FUNCTION("get_civilization_planet", [&] (entt::entity civ) {
        return universe.get<cqspc::Civilization>(civ).starting_planet;
    });

    REGISTER_FUNCTION("is_player", [&] (entt::entity civ) {
        return static_cast<bool>(universe.all_of<cqspc::Player>(civ));
    });

    REGISTER_FUNCTION("add_planet_habitation", [&] (entt::entity planet) {
        universe.emplace<cqspc::Habitation>(planet);
    });

    REGISTER_FUNCTION("add_planet_settlement", [&](entt::entity planet,
                                                   double lat, double longi) {
        entt::entity settlement = universe.create();
        universe.emplace<cqspc::Settlement>(settlement);
        // Add to planet list
        universe.get<cqspc::Habitation>(planet).settlements.push_back(settlement);
        universe.emplace<cqspt::SurfaceCoordinate>(settlement, lat, longi);
        return settlement;
    });

    REGISTER_FUNCTION("add_population_segment", [&](entt::entity settlement, uint64_t popsize) {
        entt::entity population = universe.create();
        universe.emplace<cqspc::PopulationSegment>(population, popsize);
        universe.emplace<cqspc::ResourceStockpile>(population);
        // Add to planet list
        universe.get<cqspc::Settlement>(settlement).population.push_back(population);

        return population;
    });

    REGISTER_FUNCTION("get_segment_size", [&](entt::entity segment) {
        return universe.get<cqspc::PopulationSegment>(segment).population;
    });

    // Configure the population
    REGISTER_FUNCTION("set_name", [&](entt::entity entity, std::string name) {
        universe.emplace_or_replace<cqspc::Name>(entity, name);
    });

    REGISTER_FUNCTION("create_industries", [&](entt::entity city) {
        universe.emplace<cqspc::Industry>(city);
    });

    REGISTER_FUNCTION("create_factory", [&](entt::entity city, entt::entity recipe,
                                                                            float productivity) {
        entt::entity factory = cqsp::common::systems::actions::CreateFactory(universe,
                                                    city, recipe, productivity);
        // Factory will produce in the first tick
        universe.emplace<cqspc::Production>(factory);
        return factory;
     });

    REGISTER_FUNCTION("set_resource_consume", [&](entt::entity entity, entt::entity good, double amount) {
        auto& consumption = universe.get_or_emplace<cqspc::ResourceConsumption>(entity);
        consumption[good] = amount;
    });

    REGISTER_FUNCTION("set_resource", [&](entt::entity planet, entt::entity resource, int seed) {
        auto& dist = universe.get_or_emplace<cqspc::ResourceDistribution>(planet);
        dist[resource] = seed;
    });

    // TODO(EhWhoAmI): Will have to fix the documentation for this so that it looks neater
    auto lambda = [&]() {
        entt::entity entity = universe.create();
        auto& market = universe.emplace<cqspc::Market>(entity);
        universe.emplace<cqspc::ResourceStockpile>(entity);
        // Set the market prices
        // TODO(EhWhoAmI): This is a bandaid solution, please fix this

        auto view = universe.view<cqspc::Good, cqspc::Price>();
        for (entt::entity entity : view) {
            // Assign price to market
            market.prices[entity] = universe.get<cqspc::Price>(entity);
        }
        return entity;
    };
    REGISTER_FUNCTION("create_market", lambda);

    REGISTER_FUNCTION("place_market", [&](entt::entity market, entt::entity planet) {
        universe.emplace<cqspc::MarketCenter>(planet, market);
    });

    REGISTER_FUNCTION("attach_market", [&](entt::entity market_entity, entt::entity participant) {
        cqsp::common::systems::economy::AddParticipant(universe, market_entity, participant);
        auto& wallet = universe.get_or_emplace<cqspc::Wallet>(participant);
    });

    REGISTER_FUNCTION("add_cash", [&](entt::entity participant, double balance) {
        universe.get_or_emplace<cqspc::Wallet>(participant).balance += balance;
    });

    REGISTER_FUNCTION("to_human_string", [&](int64_t number) {
        return cqsp::util::LongToHumanString(number);
    });

    REGISTER_FUNCTION("add_resource", [&](entt::entity storage, entt::entity resource, int amount) {
        // Add resources to the resource stockpile
        universe.get<cqspc::ResourceStockpile>(storage)[resource] += amount;
    });

    REGISTER_FUNCTION("create_mine", [&](entt::entity city, entt::entity resource, int amount, float productivity) {
        return cqsp::common::systems::actions::CreateMine(universe, city, resource, amount);
    });

    REGISTER_FUNCTION("create_terrain", [&](entt::entity planet, int seed) {
        universe.emplace<cqspb::Terrain>(planet, seed);
    });

    REGISTER_FUNCTION("create_ship", [&](entt::entity civ, entt::entity orbit, entt::entity starsystem) {
        return cqsp::common::systems::actions::CreateShip(universe, civ, orbit, starsystem);
    });

    REGISTER_FUNCTION("get_player", [&]() {
        return universe.view<cqspc::Player>().front();
    });

    REGISTER_FUNCTION("get_all_civs", [&]() {
        return sol::as_table(universe.view<cqspc::Civilization>());
    });

    // Get cities of a planet
    REGISTER_FUNCTION("get_cities", [&](entt::entity planet) {
        return universe.get<cqspc::Habitation>(planet).settlements;
    });

    REGISTER_FUNCTION("get_resource_count", [&](entt::entity stockpile, entt::entity resource) {
        return universe.get<cqspc::ResourceStockpile>(stockpile)[resource];
    });

    // Get population segments of a planet
    REGISTER_FUNCTION("get_segments", [&](entt::entity planet) {
        return universe.get<cqspc::Settlement>(planet).population;
    });

    // Get population segments of a planet
    REGISTER_FUNCTION("get_name", [&](entt::entity entity) {
        return universe.get<cqspc::Name>(entity).name;
    });

    REGISTER_FUNCTION("push_event", [&](entt::entity entity, sol::table event_table) {
        auto& queue = universe.get_or_emplace<cqsp::common::event::EventQueue>(entity);
        auto event = std::make_shared<cqsp::common::event::Event>();
        event->title = event_table["title"];
        SPDLOG_INFO("Parsing event \"{}\"", event->title);
        event->content = event_table["content"];
        event->image = event_table["image"];
        sol::optional<std::vector<sol::table>> optional = event_table["actions"];
        if (optional) {
            for (auto &action : *optional) {
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
