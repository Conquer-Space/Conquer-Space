/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include "common/components/coordinates.h"
#include "common/components/orbit.h"
#include "common/universe.h"

namespace cqsp::common::systems::loading {
components::types::Orbit GetOrbit(const std::string& line_one, const std::string& line_two, const double& GM);
int GetEpochYear(int year);
double GetEpoch(double year, double time);
void LoadSatellites(Universe& universe, std::string& string);
}  // namespace cqsp::common::systems::loading
