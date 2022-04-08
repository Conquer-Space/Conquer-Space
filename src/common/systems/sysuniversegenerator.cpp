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
#include "common/systems/sysuniversegenerator.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>

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

#include "common/systems/actions/factoryconstructaction.h"
#include "common/systems/economy/markethelpers.h"
#include "actions/shiplaunchaction.h"
#include "client/systems/views/starsystemview.h"

void cqsp::common::systems::universegenerator::ScriptUniverseGenerator::
    Generate(cqsp::common::Universe& universe) {
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    namespace cqspc = cqsp::common::components;

    script_engine["goods"] = universe.goods;
    script_engine["recipes"] = universe.recipes;
    script_engine["terrain_colors"] = universe.terrain_data;
    // Create player
    auto player = universe.create();
    universe.emplace<cqspc::Civilization>(player);
    universe.emplace<cqspc::Player>(player);
    // Add wallet to civilization
    universe.emplace<cqspc::Wallet>(player, entt::null, 10000000);

    // Add top level fleet
    auto playerFleet = universe.create();
    universe.emplace<cqspc::Name>(playerFleet, "navy");
    universe.emplace<cqspc::ships::Fleet>(playerFleet, player);
    universe.get<cqspc::Civilization>(player).topLevelFleet = playerFleet;
    // Add a subfleet
    auto playerSubFleet = universe.create();
    universe.emplace<cqspc::Name>(playerSubFleet, "vice-navy");
    universe.emplace<cqspc::ships::Fleet>(playerSubFleet, playerFleet, player, 1);
    universe.get<cqsps::Fleet>(universe.get<cqspc::Civilization>(player).topLevelFleet)
        .subFleets.push_back(playerSubFleet);
    //for (int i = 0; i < 9; i++) {
        //auto civ = universe.create();
        //universe.emplace<cqspc::Civilization>(civ);
    //}
    sol::optional<sol::table> generator = script_engine["generators"]["data"][1];
    if (generator) {
        (*generator)["civ_init"]();
        script_engine["civilizations"] = sol::as_table(universe.view<cqspc::Civilization>());
        (*generator)["universe_gen"]();
        auto view = universe.view<cqspc::Civilization>();
        for (auto ent : view) {
            (*generator)["planets"](ent);
        }
    }
    // add first ship(could be deferred to some script)
    //has to be deferred until after the galaxy and systems are populated in the scripts
    auto starting_planet =
        universe.get<cqspc::Civilization>(player).starting_planet;
    cqsp::common::systems::actions::CreateShip(
        universe, playerFleet,
        universe.get<cqspc::bodies::Body>(starting_planet).star_system,
        starting_planet,
        "pioneer");
      cqsp::common::systems::actions::CreateShip(
        universe, playerSubFleet,
        universe.get<cqspc::bodies::Body>(starting_planet).star_system,
        starting_planet,
        "pioneer2");
}
