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

#include <math.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "common/components/bodies.h"
#include "common/components/units.h"

namespace cqsp::common::components::types {
/// Relative position from the parent orbiting object
struct Kinematics {
    glm::dvec3 position {0, 0, 0};
    glm::dvec3 velocity {0, 0, 0};
    glm::dvec3 center {0, 0, 0};
};

struct FuturePosition {
    glm::dvec3 position {0, 0, 0};
    glm::dvec3 velocity {0, 0, 0};
    glm::dvec3 center {0, 0, 0};
    double true_anomaly;
};

// A one tick impulse in the vector
struct Impulse {
    glm::dvec3 impulse;
};

/// <summary>
/// Way to position star systems on the universe.
/// </summary>
struct GalacticCoordinate {
    types::light_year x;
    types::light_year y;
};

template <typename T>
struct PolarCoordinate_tp {
    T r;
    degree theta;

    PolarCoordinate_tp() = default;
    PolarCoordinate_tp(T _r, degree _theta) : r(_r), theta(_theta) {}
};
typedef PolarCoordinate_tp<types::astronomical_unit> PolarCoordinate;

struct MoveTarget {
    entt::entity target;
    explicit MoveTarget(entt::entity _targetent) : target(_targetent) {}
};

/// <summary>
/// Longitude and lattitude.
/// Planet coordinates.
/// </summary>
struct SurfaceCoordinate {
 private:
    radian _latitude;
    radian _longitude;

 public:
    SurfaceCoordinate() = default;

    /// <summary>
    /// Creates a surface coordinate
    /// </summary>
    /// <param name="_lat">Latitude in degrees (-90 ~ 90)</param>
    /// <param name="_long">Longtitude in degrees(-180 ~ 180)</param>
    SurfaceCoordinate(degree _lat, degree _long) : _latitude(toRadian(_lat)), _longitude(toRadian(_long)) {
        _lat = normalize_radian_coord(_lat);
        _longitude = normalize_radian_coord(_longitude);
    }

    SurfaceCoordinate(radian _lat, radian _long, bool radian)
        : _latitude(normalize_radian_coord(_lat)), _longitude(normalize_radian_coord(_long)) {}

    degree latitude() const { return toDegree(_latitude); }

    degree longitude() const { return toDegree(_longitude); }

    radian r_longitude() const { return _longitude; }

    radian r_latitude() const { return _latitude; }

    SurfaceCoordinate universe_view() const { return SurfaceCoordinate(_latitude, _longitude, true); }

    entt::entity planet = entt::null;
};

/// <summary>
/// Calculates the angle between the two coordinates. Just multiply by the planet's diameter for fun and engagement.
/// </summary>
/// <param name="coord1"></param>
/// <param name="coord2"></param>
/// <returns></returns>
double GreatCircleDistance(SurfaceCoordinate& coord1, SurfaceCoordinate& coord2);

/// <summary>
/// Converts surface coordinate to vector3 in space so that we can get the surface
/// coordinate to render in space
/// </summary>
glm::vec3 toVec3(const SurfaceCoordinate& coord, const float& radius = 1);

/// <summary>
/// 2D polar coordinate to opengl 3d coordinate
/// </summary>
/// <param name="coordinate"></param>
/// <returns></returns>
inline glm::vec3 toVec3(const PolarCoordinate& coordinate) {
    return glm::vec3(coordinate.r * cos(toRadian(coordinate.theta)), 0, coordinate.r * sin(toRadian(coordinate.theta)));
}

/// <summary>
/// Calculates the azimuth of the orbit based on the latitude and inclination
/// </summary>
/// <param name="latitude">Latitude in radians</param>
/// <param name="inclination">Launch inclination in radians</param>
/// <returns></returns>
double GetLaunchAzimuth(double latitude, double inclination);

/// <summary>
/// Calculates the inclination of the orbit based on the latitude and azimuth
/// </summary>
/// <param name="latitude">Latitude in radians</param>
/// <param name="azimuth">Launch azimuth in radians</param>
/// <returns></returns>
double GetLaunchInclination(double latitude, double azimuth);

SurfaceCoordinate ToSurfaceCoordinate(const glm::vec3& vec);
}  // namespace cqsp::common::components::types
