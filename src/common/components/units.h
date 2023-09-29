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

#include <cmath>

namespace cqsp::common::components::types {
enum UnitType {
    Distance,  // Default distance: kilometer
    Angle,     // Default distance: degree
    Mass,      // Default distance: kilogram
    Volume,    // Default distance: m^3
    Time       // Default distance: second
};

enum Distance { LightYear, AstronomicalUnit, Kilometer, Meter, Centimeter, Millimeter };

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

// Constants
constexpr double PI = 3.14159265358979323846;
constexpr double TWOPI = PI * 2;
constexpr double HALFPI = PI / 2;

constexpr double KmInAu = 149597870.700;

// Gravitional constant in m^3 * kg^-1 * s^-2
constexpr double G = 6.6743015e-11;
// Gravitional constant in km^3 * kg^-1 * s^-2
constexpr double G_km = 6.6743015e-20;

// GM of sun/sun gravitational constant in km^3 * s^-2
constexpr double SunMu = 1.32712400188e11;

#if __cplusplus == 202302L
// Then use fmod
#define floatmod std::fmod
#else
inline double constexpr floatmod(double x, double y) { return x - y * (int)(x / y); }
#endif

/// <summary>
/// Normalizes a radian to [0, PI*2)
/// </summary>
/// \param[in] Radian to normalizes
inline constexpr radian normalize_radian(const radian& radian) {
    double x = floatmod(radian, TWOPI);
    if (x < 0) {
        x += TWOPI;
    }
    return x;
}

inline constexpr double normalize_radian_coord(const radian& radian) {
    double r = floatmod(radian + PI, TWOPI);
    if (r < 0) r += TWOPI;
    return r - PI;
}

inline constexpr degree normalize_degree(const degree& radian) {
    double x = floatmod(radian, 360);
    if (x < 0) {
        x += 360;
    }
    return x;
}

// Conversions
inline constexpr astronomical_unit toAU(kilometer km) { return km / KmInAu; }
inline constexpr light_year toLightYear(astronomical_unit au) { return au / 63241; }
inline constexpr astronomical_unit LtyrtoAU(light_year ltyr) { return ltyr * 63241; }
inline constexpr kilometer toKm(astronomical_unit au) { return au * KmInAu; }
inline constexpr radian toRadian(degree theta) { return theta * (PI / 180.f); }
inline constexpr degree toDegree(radian theta) { return theta * (180.f / PI); }

inline constexpr double operator""_deg(const long double deg) { return normalize_radian(toRadian(deg)); }
inline constexpr double operator""_au(const long double au) { return toKm(au); }
#ifdef floatmod
#undef floatmod
#endif  // floatmod
}  // namespace cqsp::common::components::types
