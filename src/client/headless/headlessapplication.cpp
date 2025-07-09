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
#include "client/headless/headlessapplication.h"

#include <iostream>
#include <string>

#include "client/headless/generate.h"
#include "common/util/logging.h"
#include "common/util/strip.h"

namespace cqsp::headless {
cqsp::asset::AssetManager& HeadlessApplication::GetAssetManager() { return asset_manager; }

cqsp::client::ConquerSpace& HeadlessApplication::GetGame() { return conquer_space; }

HeadlessApplication::HeadlessApplication() : asset_manager(), asset_loader(asset::AssetOptions(false)) {}

int HeadlessApplication::run() {
    // Load data
    cqsp::engine::engine_logger = cqsp::common::util::make_logger("app", true);
    auto g_logger = cqsp::common::util::make_logger("game", true);
    spdlog::set_default_logger(g_logger);

    std::cout << "Loading game data...\n";
    asset_loader.manager = &asset_manager;
    asset_loader.LoadMods();

    // Now let's generate a universe
    while (1) {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);
        // Now we parse the command line depending on the mode
        if (!line.empty() && line[0] == '@') {
            // Then we have a special command or something
            // Now we get the commands
            if (line == "@generate") {
                generate(*this);
            }
        }
    }
    return 0;
}
}  // namespace cqsp::headless
