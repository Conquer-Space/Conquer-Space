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

#include <string>

namespace cqsp::core::util {
/// <summary>
/// The path for where all the data of the game is stored, including logs, and other things
/// </summary>
/// <returns></returns>
std::string GetCqspAppDataPath();
std::string GetCqspExePath();
/// <summary>
///  This returns binaries/data
/// </summary>
/// <returns></returns>
std::string GetCqspDataPath();
std::string GetCqspTestDataPath();
/// This should return Documents/cqsp
std::string GetCqspSavePath();
}  // namespace cqsp::core::util
