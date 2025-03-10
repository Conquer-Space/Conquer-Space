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
#include "common/systems/sysuniversegenerator.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string>

#include "actions/shiplaunchaction.h"
#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/systems/actions/factoryconstructaction.h"
#include "common/systems/economy/markethelpers.h"
#include "common/util/random/stdrandom.h"

namespace cqsp::common::systems::universegenerator {
void ScriptUniverseGenerator::Generate(cqsp::common::Universe& universe) {
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;
    namespace cqspc = cqsp::common::components;

    script_engine["goods"] = universe.goods;
    script_engine["recipes"] = universe.recipes;
    script_engine["terrain_colors"] = universe.terrain_data;
    script_engine["fields"] = universe.fields;
    script_engine["technologies"] = universe.technologies;
    script_engine["countries"] = universe.countries;
    SPDLOG_INFO("Set goods");
    // Create player
    // Set player
    // Set to country
    auto player = universe.countries["usa"];
    //universe.emplace<cqspc::Civilization>(player);
    universe.emplace<cqspc::Player>(player);

    // Add top level fleet
    /*
    auto playerFleet = universe.create();
    universe.emplace<cqspc::Name>(playerFleet, "navy");
    universe.emplace<cqspc::ships::Fleet>(playerFleet, player);
    universe.get<cqspc::Civilization>(player).top_level_fleet = playerFleet;
    // Add a subfleet
    auto playerSubFleet = universe.create();
    universe.emplace<cqspc::Name>(playerSubFleet, "vice-navy");
    universe.emplace<cqspc::ships::Fleet>(playerSubFleet, playerFleet, player, 1);
    universe.get<cqsps::Fleet>(universe.get<cqspc::Civilization>(player).top_level_fleet)
        .subfleets.push_back(playerSubFleet);
        */
    /*
    sol::optional<sol::table> generator = script_engine["generators"]["data"][1];
    if (generator) {
        (*generator)["civ_init"]();
        //script_engine["civilizations"] = sol::as_table(universe.view<cqspc::Civilization>());
        (*generator)["universe_gen"]();
        auto view = universe.view<cqspc::Civilization>();
        SPDLOG_INFO("Initing planets");
        for (auto ent : view) {
            (*generator)["planets"](ent);
        }
    } else {
        SPDLOG_ERROR("No generator");
    }*/
    SPDLOG_INFO("Done generating");
}
}  // namespace cqsp::common::systems::universegenerator
