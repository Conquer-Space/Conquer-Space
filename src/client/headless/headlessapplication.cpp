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

#include <sol/error.hpp>

#include "client/headless/generate.h"
#include "client/headless/headlessluafunctions.h"
#include "client/headless/loadluafile.h"
#include "common/util/logging.h"
#include "common/util/string.h"

namespace cqsp::client::headless {

using common::systems::simulation::Simulation;

asset::AssetManager& HeadlessApplication::GetAssetManager() { return asset_manager; }

ConquerSpace& HeadlessApplication::GetGame() { return conquer_space; }

HeadlessApplication::HeadlessApplication() : asset_loader(asset::AssetOptions(false)) {}

int HeadlessApplication::run() {
    // Load data
    engine::engine_logger = common::util::make_logger("app", true);
    auto g_logger = common::util::make_logger("game", true);
    spdlog::set_default_logger(g_logger);

    std::cout << "Loading game data...\n";
    asset_loader.manager = &asset_manager;
    asset_loader.LoadMods();

    // Add lua functions
    LoadHeadlessFunctions(*this);

    while (true) {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);
        // Now we parse the command line depending on the mode
        if (!line.empty() && (line[0] == '@' || (line[0] == '-' && line[1] == '-'))) {
            // Let's split the arguments
            // Let's just get the first space
            std::vector<std::string> arguments;
            if (line.find(' ') != std::string::npos) {
                std::string arg_string = line.substr(line.find(' '), std::string::npos);
                arg_string = util::strip(arg_string);
                // Now we have to iterate forward and find any quotation marks...
                arguments = util::split(arg_string, " ");
                line = line.substr(0, line.find(' '));
            }
            if (IsCommandComment(line, arguments, "generate")) {
                generate(*this);
                // Now generate the simulation
            } else if (IsCommandComment(line, arguments, "loadluafile")) {
                loadluafile(*this, arguments);
            } else if (IsCommandComment(line, arguments, "exit")) {
                break;
            } else if (line != "--") {
                // Then it doesn't exist
                std::cout << "Unknown command \'" << line << "\'!\n";
            }
        } else {
            // Lua scripting
            // TODO(#282): Print out variable if it's not assigned to something
            try {
                conquer_space.GetScriptInterface().RunScript(line);
            } catch (sol::error& error) {
                // since it's automatically logged we can ignore it
                ;
            }
        }
    }
    return 0;
}

/*
* Initializes the pointer for the simulation
*/
void HeadlessApplication::InitSimulationPtr() {
    // I am not happy with this interface
    simulation = std::make_unique<Simulation>(GetGame().GetGame());
}

Simulation& HeadlessApplication::GetSimulation() { return *(simulation); }

bool HeadlessApplication::IsCommandComment(const std::string& line, const std::vector<std::string>& arguments,
                                           const std::string& command) {
    if (line != "--") {
        return false;
    }
    if (line == ("@" + command)) {
        return true;
    }
    if (arguments.empty()) {
        return false;
    }
    return (arguments[0] == command);
}
}  // namespace cqsp::client::headless
