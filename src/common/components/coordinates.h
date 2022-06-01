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
    /// Average motion
    /// </summary>
    double nu;

    /// <summary>
    /// Graviational constant * mass of orbiting body
    /// km^3 * s^-2
    /// </summary>
    double Mu = SunMu;

    // So we can prepare for moons and stuff
    entt::entity reference_body = entt::null;

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

    /// <summary>
    /// Calculates period and mean motion
    /// </summary>
    void CalculatePeriod() {
        T =  2 * PI * std::sqrt(semi_major_axis * semi_major_axis * semi_major_axis / Mu);
        nu = std::sqrt(Mu / (semi_major_axis * semi_major_axis * semi_major_axis));
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
    if (a == 0) {
        return glm::vec3(0, 0, 0);
    }
    double r = (a * (1 - e * e)) / (1 + e * cos(v));
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
    return glm::vec3{-rx, -rz, ry};
}

inline glm::vec3 OrbitVelocityToVec3(const double& a, const double& e, const radian& i,
                             const radian& LAN, const radian& w, const radian& v) {

}
    /// <summary>
/// Computes eccentric anomaly in radians given mean anomaly and eccentricity
/// </summary>
/// <param name="mean_anomaly"></param>
/// <param name="ecc"></param>
/// <returns></returns>
static double SolveKepler(const double& mean_anomaly, const double& ecc, const int steps = 200) {
    // Because math is hard, we need to solve it iteratively
    const int size = 10;
    if (abs(ecc) < 1.0E-9) {
            return mean_anomaly;
    }
    int maxit = steps;
    int it = 0;

    double de = 1000.0;

    double ea = mean_anomaly;
    double old_m = mean_anomaly;

    while ((it < maxit) && (abs(de) > 1.0E-5)) { // normal accuracy is 1.0e-10
        double new_m = ea - ecc * sin(ea);
        de = (old_m - new_m) / (1.0 - ecc * cos(ea));
        ea = ea + de;
        it = it + 1;
    }
    return ea;
}

/// <param name="orbit">Orbit to compute</param>
/// <param name="time">Current time (seconds)</param>
/// <returns>True anomaly in radians</returns>
static radian TrueAnomaly(const Orbit& orbit, const second& time) {
    // Calculate
    double Mt = orbit.M0 + time * orbit.nu;
    normalize_radian(Mt);
    double E = SolveKepler(Mt, orbit.eccentricity);
    return 2 * atan2 (sqrt(1 + orbit.eccentricity) * sin (E / 2), sqrt(1 - orbit.eccentricity) * cos (E / 2));
}

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

/// <summary>
/// Updates the orbit's true anomaly
/// </summary>
/// <param name="orb"></param>
/// <param name="time"></param>
inline void UpdateOrbit(Orbit& orb, const second& time) {
    orb.v = TrueAnomaly(orb, time);
}

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
    /// <param name="_lat">Latitude in degrees</param>
    /// <param name="_long">Longtitude in degrees</param>
    SurfaceCoordinate(degree _lat, degree _long)
        : _latitude(toRadian(_lat)), _longitude(toRadian(_long)) {
        _lat = normalize_radian(_lat);
        _longitude = normalize_radian(_longitude);
    }

    degree latitude() const {
        return toDegree(_latitude);
    }

    degree longitude() const {
        return toDegree(_longitude);
    }

    radian r_longitude() const { return _longitude; }

    radian r_latitude() const { return _latitude; }
};

/// <summary>
/// Converts surface coordinate to vector3, shown in opengl.
/// </summary>
inline glm::vec3 toVec3(const SurfaceCoordinate& coord, const float& radius = 1) {
    return glm::vec3(cos(coord.r_latitude()) * sin(coord.r_longitude()),
                     sin(coord.r_latitude()),
                     cos(coord.r_latitude()) * cos(coord.r_longitude())) * radius;
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

/// <summary>
/// Convert orbit to AU coordinates
/// </summary>
/// <param name="orb">Orbit</param>
/// <param name="theta">Theta to compute</param>
/// <returns>Vector 3 in orbit, in AU, configured for opengl</returns>
inline Vec3AU toVec3AU(const Orbit& orb, radian theta) {
    glm::vec3 vec = OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
    return vec/(float) KmInAu;
}

inline glm::vec3 toVec3(const Orbit& orb, radian theta) {
    return OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
}

inline glm::vec3 toVec3(const Orbit& orb) {
    return toVec3(orb, orb.v);
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
/// Updates the position of an orbit, in AU.
/// </summary>
/// <param name="kin">[out] the kinematics of the orbit</param>
/// <param name="orb">[in] the orbit of the orbit</param>
inline void UpdatePos(Kinematics& kin, const Orbit& orb) {
    // Calculate time
    kin.position = toVec3AU(orb);
}
}  // namespace cqsp::common::components::types
