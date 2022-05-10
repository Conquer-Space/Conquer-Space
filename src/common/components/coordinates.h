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

#include <spdlog/spdlog.h>  // remove 
#include <math.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "common/components/bodies.h"
#include "common/components/units.h"
#include <iostream> // remove 
namespace cqsp {
namespace common {
namespace components {
namespace types {

struct Orbit;
inline double FindAngularVelocity(const Orbit& orb);

const static double G = 6.6743015e-11;
const static double SunMu = 1.32712400188e11;

/**
 * Orbit of a body
 */
struct Orbit {
    double eccentricity; // e
    double semi_major_axis; // a
    double inclination; // i
    double ascending_node; // capital Omega (Longitude of the ascending node)
    double argument; // lower case omega
    double anomaly; // theta
    double epoch; // t0

    double T;

    // So we can prepare for moons and stuff
    entt::entity referenceBody = entt::null;

    Orbit() = default;
    Orbit(double eccentricity, double semi_major_axis, double inclination, double ascending_node, double argument, double anomaly, double epoch):
            eccentricity(eccentricity),
            semi_major_axis(semi_major_axis),
            inclination(inclination),
            ascending_node(ascending_node),
            argument(argument),
            anomaly(anomaly),
            epoch(epoch) {
        // Assume it's orbiting the sun
        T = CalculatePeriod();
    }

    double CalculatePeriod() {
        return 2 * PI *
            std::sqrt(semi_major_axis * semi_major_axis *
                      semi_major_axis / SunMu);
    }
};

struct Kinematics {
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    float topspeed = 0.1;
};

/// <summary>
/// Way to position star systems on the universe.
/// </summary>
struct GalacticCoordinate {
    types::light_year x;
    types::light_year y;
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
    return 0;  // TWOPI * std::sqrt(std::pow(orb.semiMajorAxis, 3) /
               // orb.gravitationalparameter)/3600.f;
}

// Angular velocity in radians per hour
inline double FindAngularVelocity(const Orbit& orb) {
    return 360.0 * (1.0 / FindPeriod(orb));
}

inline void UpdateOrbit(Orbit& orb) {
    orb.anomaly += 0.1;  // FindAngularVelocity(orb);
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
    /*double r = orb.semiMajorAxis *
                  (1 - orb.eccentricity * orb.eccentricity) /
               (1 - orb.eccentricity *
                        cos(toRadian(fmod(orb.theta, 360) + orb.argument)));*/
    return PolarCoordinate{1, 1};
}

inline glm::vec3 toVec3(const Orbit& orb) {
    // Convert the orbit
    double mu = SunMu;
    double Mt; // mean anomaly at t
    // Orbital period
    if (orb.anomaly == orb.epoch) {
        Mt = orb.anomaly;
    } else {
        double deltaT = (orb.T) * (orb.anomaly - orb.epoch); //divide time increments into 1/60th of the orbital period
        std::cout << deltaT << std::endl;
        Mt = deltaT * std::sqrt(mu / (orb.semi_major_axis * orb.semi_major_axis * orb.semi_major_axis));
    }

    double E = Mt;
    double F = E - orb.eccentricity * std::sin(E) - Mt;
    int j = 0, maxIter = 30;
    double delta = 0.000001f;
    while (std::abs(F) > delta && j < maxIter) {
        E = E - F / (1 - orb.eccentricity * std::cos(E));
        F = E - orb.eccentricity * sin(E) - Mt;
        j++;
    }

    double nu = 2 * std::atan2(sqrt(1 + orb.eccentricity) *
                                  sin(orb.eccentricity / 2), sqrt (1 - orb.eccentricity) * cos(E / 2));
    double rc = orb.semi_major_axis * (1 - orb.eccentricity * cos(E));
    glm::dvec3 o{rc * cos(nu), rc * sin(nu), 0};
    glm::dvec3 odot{sin(E), sqrt(1 - orb.eccentricity * orb.eccentricity) * cos(E), 0};
    odot *= (sqrt(mu * orb.semi_major_axis) / rc);

    double rx, ry, rz;
    rx = o.x;
    ry = o.y;
    rz = o.z;

    rx = ( o.x * (cos(orb.argument) * cos(orb.ascending_node) - sin(orb.argument) * cos(orb.inclination) * sin(orb.ascending_node)) -
            o.y * (sin(orb.argument) * cos(orb.ascending_node) + cos(orb.argument) * cos(orb.inclination) * sin(orb.ascending_node)));
    ry = (o.x * (cos (orb.argument) * sin(orb.ascending_node) + sin(orb.argument) * cos(orb.inclination) * cos(orb.ascending_node)) +
        o.y * (cos(orb.argument) * cos(orb.inclination) * cos(orb.ascending_node) - sin(orb.argument) * sin(orb.ascending_node)));
    rz = (o.x * (sin(orb.argument) * sin(orb.inclination)) + o.y * (cos(orb.argument) * sin(orb.inclination)));
    return glm::vec3(rx, ry, rz);
}

inline void UpdatePos(Kinematics& kin, const Orbit& orb) {
    kin.position = toVec3(orb);
}
}  // namespace types
}  // namespace components
}  // namespace common
}  // namespace cqsp
