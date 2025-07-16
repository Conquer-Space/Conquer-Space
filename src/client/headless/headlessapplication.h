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
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "client/conquerspace.h"
#include "common/simulation.h"
#include "common/universe.h"
#include "engine/asset/assetloader.h"
#include "engine/asset/assetmanager.h"

namespace cqsp::client::headless {

typedef int (*HeadlessCommand)(ConquerSpace&, std::vector<std::string> arguments);

class HeadlessApplication {
 public:
    HeadlessApplication();
    int run();

    asset::AssetManager& GetAssetManager();
    ConquerSpace& GetGame();

    void InitSimulationPtr();
    common::systems::simulation::Simulation& GetSimulation();

 private:
    asset::AssetManager asset_manager;
    asset::AssetLoader asset_loader;

    client::ConquerSpace conquer_space;
    std::unique_ptr<common::systems::simulation::Simulation> simulation;

    std::map<std::string, HeadlessCommand> command_map;
};
};  // namespace cqsp::client::headless
