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
#include "client/systems/universeloader.h"

#include "client/systems/assetloading.h"
#include "common/systems/sysuniversegenerator.h"
#include "common/util/save/savegame.h"

namespace cqsp::client {

using common::systems::universegenerator::ScriptUniverseGenerator;

void LoadUniverse(asset::AssetManager& asset_manager, ConquerSpace& conquer_space) {
    systems::LoadAllResources(asset_manager, conquer_space);
    SPDLOG_INFO("Made all game resources into game objects");
    using asset::TextAsset;
    // Process scripts for core
    TextAsset* script_list = asset_manager.GetAsset<TextAsset>("core:base");
    conquer_space.GetScriptInterface().RunScript(script_list->data);
    SPDLOG_INFO("Done loading scripts");
    // Load universe
    ScriptUniverseGenerator script_generator(conquer_space.GetScriptInterface());

    script_generator.Generate(conquer_space.GetUniverse());
}
}  // namespace cqsp::client
