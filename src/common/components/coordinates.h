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

#include <math.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "common/components/bodies.h"
#include "common/components/units.h"

namespace cqsp::common::components::types {

// Gravitional constant in m^3 * kg^-1 * s^-2
static const double G = 6.6743015e-11;

// Sun gravitational constant in km^3 * s^-2
static const double SunMu = 1.32712400188e11;

/// <summary>
/// A vector3 where the units are astronomical units
/// </summary>
typedef glm::vec3 Vec3AU;

/**
 * Orbit of a body
 */
struct Orbit {
    /// eccentricity
    /// Dimensionless
    /// e
    double eccentricity;

    /// Semi major axis
    /// Kilometers
    /// a
    kilometer semi_major_axis; // a

    /// inclination
    /// Radians
    /// i
    radian inclination;

    /// Longitude of the ascending node
    /// Radians
    /// Capital Omega
    radian LAN;


    /// Argument of perapsis
    /// Radians
    /// lower case omega (w)
    radian w;

    // Mean anomaly at epoch (J2000)
    // Radians
    // M sub 0
    radian M0;

    /// <summary>
    /// True anomaly
    /// v
    /// Radians
    /// </summary>
    double v;

    /// <summary>
    /// Orbital period
    /// Seconds
    /// </summary>
    double T;

    /// <summary>
    /// Graviational constant * mass of orbiting body
    /// km^3 * s^-2
    /// </summary>
    double Mu;

    // So we can prepare for moons and stuff
    entt::entity referenceBody = entt::null;

    Orbit() = default;
    Orbit(kilometer semi_major_axis, double eccentricity,
            radian inclination, radian LAN,
            radian w, radian M0):
            eccentricity(eccentricity),
            semi_major_axis(semi_major_axis),
            inclination(inclination),
            LAN(LAN),
            w(w),
            M0(M0),
            v(M0),
            T(0) {
        CalculatePeriod();
    }

    void CalculatePeriod() {
        T =  2 * PI * std::sqrt(semi_major_axis * semi_major_axis * semi_major_axis / Mu);
    }
};

/// <summary>
/// Converts an orbit to a vec3.
/// </summary>
/// <param name="a">Semi major axis (unit doesn't matter)</param>
/// <param name="e">Eccentricity (Dimensionless)</param>
/// <param name="i">Inclinations (radians)</param>
/// <param name="LAN">Longitude of ascending node (radians)</param>
/// <param name="w">Argument of periapsis (radians)</param>
/// <param name="v">True anomaly (radians)</param>
/// <returns>The vec3, in whatever unit a was.</returns>
inline glm::vec3 OrbitToVec3(const double& a, const double& e, const radian& i,
                             const radian& LAN, const radian& w, const radian& v) {
    // Calculate the things for now
    double r = (a) / (1 - e * cos(v));
    double x = r * cos(v);
    double y = r * sin(v);
    double z = 0;
    glm::dvec3 o{x, y, z};

    double rx = (o.x * (cos(w) * cos(LAN) - sin(w) * cos(i) * sin(LAN)) -
            o.y * (sin(w) * cos(LAN) + cos(w) * cos(i) * sin(LAN)));
    double ry = (o.x * (cos(w) * sin(LAN) + sin(w) * cos(i) * cos(LAN)) +
        o.y * (cos(w) * cos(i) * cos(LAN) - sin(w) * sin(LAN)));
    double rz = (o.x * (sin(w) * sin(i)) + o.y * (cos(w) * sin(i)));

    // Convert to opengl coords
    return glm::vec3{rx, -rz, ry};
}

/// <param name="orbit">Orbit to compute</param>
/// <param name="time">Current time (seconds)</param>
/// <returns>True anomaly in radians</returns>
static radian TrueAnomaly(const Orbit& orbit, second time) { return 0; }

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
    PolarCoordinate_tp(T _r, degree _theta) : r(_r), theta(_theta) {}
};
typedef PolarCoordinate_tp<types::astronomical_unit> PolarCoordinate;

struct MoveTarget {
    entt::entity target;
    explicit MoveTarget(entt::entity _targetent) : target(_targetent) {}
};

inline void UpdateOrbit(Orbit& orb) {
}

inline radian toRadian(degree theta) {
    return theta * (PI / 180.f);
}

/// <summary>
/// Longitude and lattitude.
/// Planet coordinates.
/// </summary>
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

/// <summary>
/// Converts surface coordinate to vector3, shown in opengl.
/// </summary>
inline glm::vec3 toVec3(SurfaceCoordinate coord, float radius) {
    return glm::vec3(cos(coord.latitude) * sin(coord.longitude),
                     sin(coord.latitude),
                     cos(coord.latitude) * cos(coord.longitude)) * radius;
}

inline degree toDegree(radian theta) {
    return theta * (180 / cqsp::common::components::types::PI);
}

/// <summary>
/// 2D polar coordinate to opengl 3d coordinate
/// </summary>
/// <param name="coordinate"></param>
/// <returns></returns>
inline glm::vec3 toVec3(const PolarCoordinate& coordinate) {
    return glm::vec3(coordinate.r * cos(toRadian(coordinate.theta)),
                     0,
                     coordinate.r * sin(toRadian(coordinate.theta)));
}

static const double KmInAu = 1.49597870700e8f;

/// <summary>
/// Convert orbit to AU coordinates
/// </summary>
/// <param name="orb">Orbit</param>
/// <param name="theta">Theta to compute</param>
/// <returns>Vector 3 in orbit, in AU, configured for opengl</returns>
inline Vec3AU toVec3AU(const Orbit& orb, radian theta) {
    glm::vec3 vec = OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
    return vec/1.49597870700e8f;
}

/// <summary>
/// Converts orbit to AU coorrdinates.
/// </summary>
/// <param name="orb">[in] orbit of the object to convert</param>
/// <returns>Vector 3 in orbit, in AU, configured for opengl</returns>
inline Vec3AU toVec3AU(const Orbit& orb) {
    return toVec3AU(orb, orb.v);
}


/// <summary>
/// 
/// </summary>
/// <param name="kin">[out] the kinematics of the orbit</param>
/// <param name="orb">[in] the orbit of the orbit</param>
inline void UpdatePos(Kinematics& kin, const Orbit& orb) {
    // Calculate time
    //kin.position = toVec3(orb, orb.M0);
}
}  // namespace cqsp::common::components::types
