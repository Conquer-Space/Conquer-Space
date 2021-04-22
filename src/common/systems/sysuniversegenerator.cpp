/*
* Copyright 2021 Conquer Space
*/
#include "common/systems/sysuniversegenerator.h"

#include <spdlog/spdlog.h>
#include <string>

#include <boost/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/normal_distribution.hpp>

#include <sol/sol.hpp>

#include "common/universe.h"
#include "common/components/bodies.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/player.h"

void conquerspace::systems::universegenerator::SysGenerateUniverse(
    conquerspace::engine::Application& app) {

    namespace cqspb = conquerspace::components::bodies;
    namespace cqspa = conquerspace::asset;
    cqspa::HjsonAsset* val
        = app.GetAssetManager().GetAsset<cqspa::HjsonAsset>("defaultuniversegen");

    // Initialize lua environment
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);

    components::Universe universe;

    // Init civilization script
    lua.set_function("create_star_system", [&] () {
        entt::entity ent = universe.registry.create();
        universe.registry.emplace<cqspb::StarSystem>(ent);
        return ent;
     });

    // Set print functions
    lua.set_function("print", sol::overload(
    [] (const char * y) {
        spdlog::info("* [lua]: {}", y);
    },
    [](int y) {
        spdlog::info("* [lua]: {}", y);
    },
    [](double y) {
        spdlog::info("* [lua]: {}", y);
    }));

    // RNG
    boost::mt19937 gen;
    lua.set_function("random", [&gen] (int low, int high) {
        boost::random::uniform_int_distribution<> dist(low, high);
        return dist(gen);
    });

    lua.set_function("random_normal_int", [&gen] (int mean, int sd) {
        boost::normal_distribution<> nd(mean, sd);
        boost::variate_generator<boost::mt19937&,
                           boost::normal_distribution<> > var_nor(gen, nd);
        return static_cast<int>(round(var_nor()));
    });

    lua.set_function("add_planet", [&] (entt::entity system) {
        entt::entity planet =  universe.registry.create();
        universe.registry.emplace<cqspb::Body>(planet);
        universe.registry.get<cqspb::StarSystem>(system).bodies.push_back(planet);
        return planet;
    });

    lua.set_function("add_star", [&] (entt::entity system) {
        entt::entity star =  universe.registry.create();
        universe.registry.emplace<cqspb::Star>(star);
        universe.registry.get<cqspb::StarSystem>(system).bodies.push_back(star);
        return star;
    });

    lua.set_function("set_orbit", [&] (entt::entity body,
                                            double distance,
                                            double anomaly,
                                            double eccentricity,
                                            double argument) {
        universe.registry.emplace<cqspb::Orbit>(body, anomaly, distance, eccentricity, argument);
    });

    lua.set_function("set_radius", [&] (entt::entity body, int radius) {
        universe.registry.emplace_or_replace<cqspb::Body>(body, radius);
    });

    lua.set_function("add_civilization", [&] () {
        entt::entity civ = universe.registry.create();
        universe.registry.emplace<conquerspace::components::Organization>(civ);
        return civ;
    });

    lua.set_function("set_civilization_planet", [&] (entt::entity civ, entt::entity planet) {
        universe.registry.emplace<conquerspace::components::Civilization>(civ, planet);
    });

    lua.set_function("get_civilization_planet", [&] (entt::entity civ) {
        return universe.registry.get<conquerspace::components::Civilization>(civ).starting_planet;
    });

    lua.set_function("is_player", [&] (entt::entity civ) {
        return (bool) universe.registry.all_of<conquerspace::components::Player>(civ);
    });

    // Load and run utility scripts
    for (int i = 0; i < val->data["utility"].size(); i++) {
        std::string utility_script = val->data[i];
        cqspa::TextAsset* civgenscriptasset =
                    app.GetAssetManager().GetAsset<cqspa::TextAsset>(utility_script);
        sol::load_result utility_result = lua.load(civgenscriptasset->data);

        sol::protected_function_result res = utility_result();
        if (res.valid()) {
            // yay!
        } else {
            // aww
            sol::error err = res;
            std::string what = err.what();
            spdlog::info("*[lua]: {}", what);
        }
    }

    // Create player
    auto player = universe.registry.create();
    universe.registry.emplace<conquerspace::components::Organization>(player);
    universe.registry.emplace<conquerspace::components::Player>(player);

    for (int i = 0; i < 9; i++) {
        auto civ = universe.registry.create();
        universe.registry.emplace<conquerspace::components::Organization>(civ);
    }

    cqspa::TextAsset* civgenscript
            = app.GetAssetManager().GetAsset<cqspa::TextAsset>(val->data["civ-gen"]);
    sol::load_result civGenResult = lua.load(civgenscript->data);

    // Loop through each civilization
    auto civilizationView =
        universe.registry.view<conquerspace::components::Organization>();

    for (auto a : civilizationView) {
        lua.set("civ", a);
        sol::protected_function_result res = civGenResult();
        if (res.valid()) {
            // yay!
        } else {
            // aww
            sol::error err = res;
            std::string what = err.what();
            spdlog::info("*[lua]: {}", what);
        }
    }

    lua.set("civilizations", sol::as_table(civilizationView));

    cqspa::TextAsset* as
            = app.GetAssetManager().GetAsset<cqspa::TextAsset>(val->data["universe-gen"]);
    sol::load_result script = lua.load(as->data);

    sol::protected_function_result result = script();
    if (result.valid()) {
        // yay!
    } else {
        // aww
        sol::error err = result;
        std::string what = err.what();
        spdlog::info("*[lua]: {}", what);
    }

    // Now add civilization
    cqspa::TextAsset* civgen =
        app.GetAssetManager().GetAsset<cqspa::TextAsset>(val->data["civ-system"]);
    sol::load_result civGenScript = lua.load(civgen->data);

    for (auto ent : civilizationView.each()) {
        lua.set("civilization_id", ent);
        // Initialize, etc
        sol::protected_function_result result = civGenScript();
        if (result.valid()) {
            // yay!
        } else {
            // aww
            sol::error err = result;
            std::string what = err.what();
            spdlog::info("*[lua]: {}", what);
        }
    }
}
