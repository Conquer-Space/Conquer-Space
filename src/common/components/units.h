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

namespace cqsp {
namespace common {
namespace components {
namespace types {
typedef double astronomical_unit;
typedef double kilometer;
typedef double solar_mass;
typedef double degree;
typedef double days;
typedef double years;
typedef double radian;
typedef double meter_cube;
typedef double kilogram;

const double PI = 3.14159265358979323846;
const double TWOPI = PI * 2;

// Conversions
inline astronomical_unit toAU(kilometer km) { return km / 149597870.700; }
inline kilometer toKm(astronomical_unit au) { return au * 149597870.700; }
}  // namespace types
}  // namespace components
}  // namespace common
}  // namespace cqsp
