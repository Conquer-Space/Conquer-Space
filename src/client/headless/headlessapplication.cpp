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

#include "common/util/logging.h"

namespace cqsp::headless {
HeadlessApplication::HeadlessApplication() : asset_manager(), asset_loader(asset::AssetOptions(false)) {}

int HeadlessApplication::run() {
    // Load data
    cqsp::engine::engine_logger = cqsp::common::util::make_logger("app", true);
    auto g_logger = cqsp::common::util::make_logger("game", true);
    spdlog::set_default_logger(g_logger);

    std::cout << "Loading game data...\n";
    asset_loader.manager = &asset_manager;
    asset_loader.LoadMods();

    while (1) {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);
    }
    return 0;
}
}  // namespace cqsp::headless
