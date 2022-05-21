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

namespace cqsp::common::components::types {
enum UnitType {
    Distance,  // Default distance: kilometer
    Angle,  // Default distance: degree
    Mass,  // Default distance: kilogram
    Volume,  // Default distance: m^3
    Time  // Default distance: second
};

enum Distance {
    LightYear,
    AstronomicalUnit,
    Kilometer,
    Meter,
    Centimeter,
    Millimeter
};

typedef double astronomical_unit;
typedef double light_year;
typedef double kilometer;
typedef double solar_mass;
typedef double degree;
typedef double radian;
typedef double meter_cube;
typedef double kilogram;
typedef double earth_masses;
typedef double joule;
typedef double second;

const double PI = 3.14159265358979323846;
const double TWOPI = PI * 2;

const double KmInAu = 149597870.700;

inline radian normalize_radian(const radian& radian) {
    double x = std::fmod(radian, TWOPI);
    if (x < 0) {
        x += TWOPI;
    }
    return x;
}

inline degree normalize_degree(const degree& radian) {
    double x = std::fmod(radian, 360);
    if (x < 0) {
        x += 360;
    }
    return x;
}

// Conversions
inline astronomical_unit toAU(kilometer km) { return km / KmInAu; }
inline light_year toLightYear(astronomical_unit au) { return au / 63241; }
inline astronomical_unit LtyrtoAU(light_year ltyr) { return ltyr * 63241; }
inline kilometer toKm(astronomical_unit au) { return au * KmInAu; }
inline radian toRadian(degree theta) { return theta * (PI / 180.f); }
inline degree toDegree(radian theta) { return theta * (180 / PI); }
}  // namespace cqsp::common::components::types
