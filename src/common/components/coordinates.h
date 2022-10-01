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
typedef glm::dvec3 Vec3AU;

/**
 * Orbit of a body
 */
struct Orbit {
    /// eccentricity
    /// <br />
    /// Dimensionless
    /// <br />
    /// e
    double eccentricity = 0;

    /// Semi major axis
    /// <br />
    /// Kilometers
    /// <br />
    /// a
    kilometer semi_major_axis = 0;

    /// inclination
    /// <br />
    /// Radians
    /// <br />
    /// i
    radian inclination = 0;

    /// Longitude of the ascending node
    /// <br />
    /// Radians
    /// <br />
    /// Capital Omega
    radian LAN = 0;

    /// Argument of perapsis
    /// <br />
    /// Radians
    /// <br />
    /// lower case omega (w)
    radian w = 0;

    /// Mean anomaly at epoch
    /// <br />
    /// Radians
    /// <br />
    /// M sub 0
    radian M0 = 0;

    double epoch = 0;

    /// <summary>
    /// True anomaly
    /// <br />
    /// v
    /// <br />
    /// Radians
    /// </summary>
    double v = 0;

    /// <summary>
    /// Orbital period
    /// <br />
    /// Seconds
    /// </summary>
    double T = 0;

    /// <summary>
    /// Average motion
    /// </summary>
    double nu = 0;

    /// <summary>
    /// Graviational constant * mass of orbiting body
    /// <br />
    /// km^3 * s^-2
    /// </summary>
    double GM = SunMu;

    // So we can prepare for moons and stuff
    entt::entity reference_body = entt::null;

    /// <summary>
    /// Eccentric anomaly
    /// </summary>
    double E = 0;

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
        CalculateVariables();
    }

    /// <summary>
    /// Calculates period and mean motion
    /// </summary>
    void CalculateVariables() {
        T =  2 * PI * std::sqrt(semi_major_axis * semi_major_axis * semi_major_axis / GM);
        nu = std::sqrt(GM / (semi_major_axis * semi_major_axis * semi_major_axis));
    }

    double GetMt(double time) {
        return normalize_radian(M0 + (time - epoch) * nu);
    }
};

struct OrbitDirty {};

/// <summary>
/// Converts the orbital params with the inclination, longitude of ascending node, and argument or periapsis
/// </summary>
/// All units are in radians
/// <param name="LAN">Longitude of the ascending node</param>
/// <param name="i">Inclination</param>
/// <param name="w">Argument of periapsis</param>
/// <param name="vec">Vector to convert</param>
glm::dvec3 ConvertOrbParams(const double LAN, const double i,
                                  const double w, const glm::dvec3& vec);

double GetOrbitingRadius(const double& e, const double& a,
                         const double& v);

    /// <summary>
/// Converts position and velocity to orbit.
/// </summary>
/// <param name="position">Position of the body</param>
/// <param name="velocity">Velocity of the body</param>
/// <param name="GM">G*M of the orbiting body</param>
Orbit Vec3ToOrbit(const glm::dvec3& position, const glm::dvec3& velocity,
                 const double& GM, const double& time);

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
glm::dvec3 OrbitToVec3(const double& a, const double& e, const radian& i,
                             const radian& LAN, const radian& w,
                             const radian& v);

double AvgOrbitalVelocity(const Orbit& orb);

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb, double v);

/// <summary>
/// Computes eccentric anomaly in radians given mean anomaly and eccentricity
/// </summary>
/// <param name="mean_anomaly"></param>
/// <param name="ecc"></param>
/// <returns></returns>
double SolveKepler(const double& mean_anomaly, const double& ecc,
                          const int steps = 200);

/// <summary>
/// Calculates true anomaly from eccentricity and eccentric anomaly
/// </summary>
/// \param[in] ecc The eccentricity of the orbit
/// \param[in] E The eccentric anomaly of the orbit
double CalculateTrueAnomaly(const double& ecc, const double& E);

/// <summary>
/// Gets the Mean anomaly from the time
/// </summary>
/// <param name="M0">Mean anomaly at t=0</param>
/// <param name="nu">G*M of orbiting body</param>
/// <param name="time">Current time</param>
/// <returns></returns>
double GetMt(const double& M0, const double& nu, const double& time,
             const double& epoch);

/// <param name="orbit">Orbit to compute</param>
/// <param name="time">Current time (seconds)</param>
/// <returns>True anomaly in radians</returns>
radian TrueAnomaly(const Orbit& orbit, const second& time);

/// Relative position from the parent orbiting object
struct Kinematics {
    glm::dvec3 position{0, 0, 0};
    glm::dvec3 velocity{0, 0, 0};
    glm::dvec3 center{0, 0, 0};
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
/// Updates the orbit's true anomaly.
/// </summary>
/// You need to call it before calculating anything to do with the orbit
/// <param name="orb"></param>
/// <param name="time"></param>
void UpdateOrbit(Orbit& orb, const second& time);

/// Calculates the vector velocity of the orbit
///
/// \param[in] E Eccentric anomaly
/// \param[in] r Orbiting radius
/// \param[in] GM G*M of orbiting body
/// \param[in] a Semi major axis
/// \param[in] e eccentricity
glm::vec3 CalculateVelocity(const double& E, const double& r,
                            const double& GM, const double& a,
                            const double& e);

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
        _lat = normalize_radian_coord(_lat);
        _longitude = normalize_radian_coord(_longitude);
    }

    SurfaceCoordinate(radian _lat, radian _long, bool radian)
        : _latitude(normalize_radian_coord(_lat)),
          _longitude(normalize_radian_coord(_long)) {
    }

    degree latitude() const {
        return toDegree(_latitude);
    }

    degree longitude() const {
        return toDegree(_longitude);
    }

    radian r_longitude() const { return _longitude; }

    radian r_latitude() const { return _latitude; }

    SurfaceCoordinate universe_view() const {
        return SurfaceCoordinate(_latitude, _longitude - PI/2, true);
    }

    entt::entity planet = entt::null;
};

/// <summary>
/// Converts surface coordinate to vector3, shown in opengl.
/// </summary>
glm::vec3 toVec3(const SurfaceCoordinate& coord,
                        const float& radius = 1);

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
/// <returns>Vector 3 in orbit, in AU</returns>
inline Vec3AU toVec3AU(const Orbit& orb, radian theta) {
    glm::dvec3 vec = OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
    return vec/KmInAu;
}

inline glm::dvec3 toVec3(const Orbit& orb, radian theta) {
    return OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
}

inline glm::dvec3 toVec3(const Orbit& orb) {
    return toVec3(orb, orb.v);
}
/// <summary>
/// Converts orbit to AU coorrdinates.
/// </summary>
/// <param name="orb">[in] orbit of the object to convert</param>
/// <returns>Vector 3 in orbit, in AU</returns>
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

SurfaceCoordinate ToSurfaceCoordinate(const glm::vec3& vec);
}  // namespace cqsp::common::components::types
