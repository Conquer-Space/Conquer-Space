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
#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace cqsp::common::util {
/// <summary>
/// Initializes a logger with the right outputs based on the various configurations
/// </summary>
/// <param name="name"></param>
/// <param name="error">If it wants an error output file, then it will output it as "{name}.error.txt</param>
/// <returns></returns>
std::shared_ptr<spdlog::logger> make_logger(const std::string& name, bool error = false);
std::shared_ptr<spdlog::logger> make_registered_logger(const std::string& name, bool error = false);
}  // namespace cqsp::common::util
