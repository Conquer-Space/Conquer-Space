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

#include <limits>
#include <map>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include <entt/entt.hpp>

#include "core/components/units.h"

namespace cqsp::core::components::bodies {
struct Body {
    /*
     * Radius of the body
     */
    types::kilometer radius = 0;

    /// <summary>
    /// Radius of sphere of influence
    /// rsoi = a(m/M)^2/5
    /// </summary>
    types::kilometer SOI = std::numeric_limits<double>::infinity();
    types::kilogram mass = 0;

    // gravitational constant in km^3 * s^-2
    double GM = 1;

    // Rotation period in seconds
    double rotation = 1;

    // Axial rotation
    double axial = 0.0;

    double rotation_offset = 0.0;
};

/// <summary>
/// Calculates SOI
/// </summary>
/// Make sure the units match up
/// <param name="mass">Mass of planet/body to calculate</param>
/// <param name="reference_mass">Mass of sun/reference body</param>
/// <param name="sma">Semi major axis of the planet</param>
/// <returns>SOI of planet</returns>
inline double CalculateSOI(const double& mass, const double& reference_mass, const double& sma) {
    return sma * std::pow(mass / reference_mass, 0.4);
}

/// <summary>
/// Calculates mass from gravitational constant
/// </summary>
/// Masses of bodies are described in gravitational constant because it's more accurate to describe like that
/// <param name="GM"></param>
/// <returns></returns>
inline double CalculateMass(const double& GM) { return GM / types::G_km; }

/// <summary>
/// Calculates the current planet rotation angle
/// </summary>
/// <param name="time">current time in seconds</param>
/// <param name="day_length">length of day in seconds</param>
/// <param name="offset">offset of day</param>
/// <returns>Angle the planet should be in radians</returns>
inline double GetPlanetRotationAngle(const double& time, const double& day_length, const double& offset) {
    return (time / day_length - offset) * types::TWOPI;
}

struct TexturedTerrain {
    std::string terrain_name;
    std::string normal_name;
    std::string roughness_name;
};

struct NautralObject {};

// For convenience, create a type-function that returns the first type of a parameter pack.
template <typename T, typename... Rest>
struct GetFirstTypeT {
    using Type = T;
};
template <typename... Pack>
using GetFirstType = GetFirstTypeT<Pack...>::Type;

// The `ExpandedIter` type will be recursively instantiated to implement the `ConcatIter` for a
// concatenated view.
template <typename... Ranges>
struct ExpandedIter;

template <typename R, typename... Ranges>
struct ExpandedIter<R, Ranges...> {
    using EltT = std::ranges::range_value_t<R>;
    using IterT = std::ranges::iterator_t<R>;
    R& r;
    IterT iter;
    ExpandedIter<Ranges...> rest_iter;
    ExpandedIter(R& r, Ranges&... rest) : r(r), rest_iter(rest...) { iter = r.begin(); }
    bool is_end() const {
        if (iter == r.end()) {
            return rest_iter.is_end();
        } else {
            return false;
        }
    }
    auto current() {
        if constexpr (sizeof...(Ranges) != 0) {
            if (iter == r.end()) {
                return rest_iter.current();
            }
        }
        return *iter;
    }
    void next() {
        if (iter == r.end()) {
            rest_iter.next();
        } else {
            ++iter;
        }
    }
};

template <>
struct ExpandedIter<> {
    bool is_end() const { return true; }
    void next() {}
};

template <typename... Views>
class ConcatIter {
    ExpandedIter<Views...> exp_iter;

 public:
    using EltT = std::ranges::range_value_t<GetFirstType<Views...>>;

    ConcatIter(Views&... views) : exp_iter(views...) {}
    ConcatIter(const ConcatIter&) = delete;
    ConcatIter(ConcatIter&& other) = default;
    ConcatIter& operator=(const ConcatIter&) = delete;
    ConcatIter& operator=(ConcatIter&&) = delete;

    bool is_end() const { return exp_iter.is_end(); }
    EltT operator*() { return exp_iter.current(); }
    ConcatIter& operator++() {
        exp_iter.next();
        return *this;
    }

 private:
};

struct EndIter {};

template <typename... Rs>
bool operator==(const ConcatIter<Rs...>& iter, EndIter) {
    return iter.is_end();
}

template <typename... Ranges>
class ConcatView {
    using IterT = ConcatIter<decltype(std::views::all(std::forward<Ranges>(std::declval<Ranges&>())))...>;
    std::tuple<decltype(std::views::all(std::forward<Ranges>(std::declval<Ranges&>())))...> views;

 public:
    ConcatView(Ranges&&... ranges) : views(std::views::all(std::forward<Ranges>(ranges))...) {}

    IterT begin() { return std::make_from_tuple<IterT>(views); }
    EndIter end() { return {}; };

 private:
};

// Concatenate ranges with same (or compatible?) element type.
template <typename... Rs>
    requires(std::ranges::range<Rs> && ...)
ConcatView<Rs...> concat(Rs&&... rs) {
    ConcatView<Rs...> v(std::forward<Rs>(rs)...);
    return v;
}

/// <summary>
/// An object for the children of an orbital object.
/// </summary>
struct OrbitalSystem {
    // Set the tree
    // TODO(EhWhoAmI): Large optimizations to be gained by separating bodies with a SOI to bodies without.
    // We can have one iterator that interacts with both
    std::vector<entt::entity> children;
    // Large bodies such as moons or planets
    std::vector<entt::entity> bodies;
    void push_back(const entt::entity entity) { children.push_back(entity); }
    auto all() { return concat(bodies, children); }
};

struct DirtyOrbit {};

struct Terrain {
    int seed;
    entt::entity terrain_type;
};

struct TerrainData {
    float sea_level;
    std::map<float, std::tuple<int, int, int, int>> data;
};

struct Star {};

struct Planet {};

struct LightEmitter {};
}  // namespace cqsp::core::components::bodies
