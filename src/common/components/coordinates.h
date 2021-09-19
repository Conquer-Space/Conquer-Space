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
#include <math.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "common/components/bodies.h"
#include "common/components/units.h"

namespace cqsp {
namespace common {
namespace components {
namespace types {

struct Orbit;
inline double FindAngularVelocity(const Orbit& orb);

/**
 * Orbit of a body
 */
struct Orbit {
    double gravitationalparameter;
    degree theta;
    astronomical_unit semiMajorAxis;
    double eccentricity;
    degree argument;
    double angular_velocity;

    glm::vec3 rotation = glm::vec3(0, 1, 0);

    // So we can prepare for moons and stuff
    entt::entity referenceBody = entt::null;

    Orbit() = default;
    Orbit(types::degree _trueAnomaly, types::astronomical_unit _semiMajorAxis,
          double _eccentricity, types::degree _argument, double _gravparam):
          theta(_trueAnomaly),
          semiMajorAxis(_semiMajorAxis),
          eccentricity(_eccentricity),
          argument(_argument),
          gravitationalparameter(_gravparam) {
        angular_velocity = FindAngularVelocity(*this);
    }
};

struct Kinematics {
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    float topspeed = 0.1;
};

template<typename T>
struct PolarCoordinate_tp {
    T r;
    degree theta;

    PolarCoordinate_tp() = default;
    PolarCoordinate_tp(T _r, types::degree _theta) : r(_r), theta(_theta) {}
};
typedef PolarCoordinate_tp<types::astronomical_unit> PolarCoordinate;

struct MoveTarget {
    entt::entity target;
    explicit MoveTarget(entt::entity _targetent) : target(_targetent) {}
};

// Period in hours
inline int FindPeriod(const Orbit& orb) {
    // Period in seconds
    return TWOPI * std::sqrt(std::pow(orb.semiMajorAxis, 3) / orb.gravitationalparameter)/3600.f;
}

// Angular velocity in radians per hour
inline double FindAngularVelocity(const Orbit& orb) {
    return 360.0 * (1.0 / FindPeriod(orb));
}

inline void UpdateOrbit(Orbit& orb) {
    orb.theta += FindAngularVelocity(orb);
}

inline types::radian toRadian(types::degree theta) {
    return theta * (cqsp::common::components::types::PI / 180.f);
}

struct SurfaceCoordinate {
    radian latitude;
    radian longitude;

    SurfaceCoordinate() = default;

    /// <summary>
    /// Creates a surface coordinate
    /// </summary>
    /// <param name="_lat">Latitude in degrees</param>
    /// <param name="_long">Longtitude in degrees</param>
    SurfaceCoordinate(degree _lat, degree _long)
        : latitude(toRadian(_lat)), longitude(toRadian(_long)) {}
};

inline glm::vec3 toVec3(SurfaceCoordinate coord, float radius) {
    return glm::vec3(cos(coord.latitude) * sin(coord.longitude),
                     sin(coord.latitude),
                     cos(coord.latitude) * cos(coord.longitude)) * radius;
}

inline types::degree toDegree(types::radian theta) {
    return theta * (180 / cqsp::common::components::types::PI);
}

inline glm::vec3 toVec3(const PolarCoordinate& coordinate) {
    return glm::vec3(coordinate.r * cos(toRadian(coordinate.theta)),
                     0,
                     coordinate.r * sin(toRadian(coordinate.theta)));
}

inline PolarCoordinate toPolarCoordinate(const Orbit& orb) {
    double r = orb.semiMajorAxis * (1 - orb.eccentricity * orb.eccentricity) /
               (1 - orb.eccentricity *
                        cos(toRadian(fmod(orb.theta, 360) + orb.argument)));
    return PolarCoordinate{(types::astronomical_unit)r, fmod(orb.theta, 360)};
}

inline glm::vec3 toVec3(const Orbit& orb) {
    return toVec3(toPolarCoordinate(orb));
}

inline void UpdatePos(Kinematics& kin, const Orbit& orb) {
    kin.position = toVec3(orb);
}
}  // namespace types
}  // namespace components
}  // namespace common
}  // namespace cqsp
