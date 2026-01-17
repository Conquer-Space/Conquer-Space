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
#include "core/systems/sysuniversegenerator.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string>

#include "core/actions/economy/markethelpers.h"
#include "core/actions/factoryconstructaction.h"
#include "core/actions/shiplaunchaction.h"
#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/player.h"
#include "core/components/population.h"
#include "core/components/resource.h"
#include "core/components/ships.h"
#include "core/components/surface.h"
#include "core/util/random/stdrandom.h"

namespace cqsp::core::systems::universegenerator {

namespace bodies = components::bodies;
namespace ships = components::ships;
namespace types = components::types;

void ScriptUniverseGenerator::Generate(Universe& universe) {
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
    //universe.emplace<components::Civilization>(player);
    universe.emplace<components::Player>(player);

    SPDLOG_INFO("Done generating");
}
}  // namespace cqsp::core::systems::universegenerator
