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
#include "client/headless/loadluafile.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace cqsp::headless {
int loadluafile(HeadlessApplication& application, const std::vector<std::string>& arguments) {
    // Now load the lua file and execute
    // Open the file argument
    if (arguments.empty()) {
        std::cout << "Usage:\n";
        std::cout << "\t@loadluafile [lua file name]\n";
        return 1;
    }
    // Else load the file
    std::ifstream input_file(arguments[0], std::ios::binary);
    std::stringstream buffer;
    if (!input_file.is_open()) {
        std::cout << "Failed to open file " << arguments[0] << "\n";
    }
    buffer << input_file.rdbuf();

    application.GetGame().GetScriptInterface().RunScript(buffer.str());
    return 0;
}
};  // namespace cqsp::headless
