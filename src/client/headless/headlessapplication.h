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
#include <string>
#include <vector>

#include "client/conquerspace.h"
#include "common/universe.h"
#include "engine/asset/assetloader.h"
#include "engine/asset/assetmanager.h"

namespace cqsp::headless {

typedef int (*HeadlessCommand)(cqsp::client::ConquerSpace&, std::vector<std::string> arguments);

class HeadlessApplication {
 public:
    HeadlessApplication();
    int run();

    cqsp::asset::AssetManager& GetAssetManager();
    cqsp::client::ConquerSpace& GetGame();

 private:
    cqsp::asset::AssetManager asset_manager;
    cqsp::asset::AssetLoader asset_loader;

    cqsp::client::ConquerSpace conquer_space;

    std::map<std::string, HeadlessCommand> command_map;
};
};  // namespace cqsp::headless
