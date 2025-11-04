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

#include <ostream>

#include <entt/entt.hpp>
#include <glm/vec3.hpp>

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/units.h"

namespace cqsp::common::components::types {
/// <summary>
/// A vector3 where the units are astronomical units
/// </summary>
typedef glm::dvec3 Vec3AU;

double GetOrbitingRadius(const double e, const kilometer a, const radian v);

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

    /// inclination, between [0, pi/2], [0, 1.57079633]
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
    radian v = 0;

    /// <summary>
    /// Gravitational constant of the reference body this is orbiting
    /// Graviational constant * mass of orbiting body
    /// So if it's in m^3 divide it by 10^9
    /// <br />
    /// km^3 * s^-2
    /// </summary>
    double GM = SunMu;

    // So we can prepare for moons and stuff
    entt::entity reference_body = entt::null;

    Orbit() = default;

    // TODO(EhWhoAmI): Mean anomaly is not the true anomaly, we need to convert
    // mean anomaly to true anomaly
    Orbit(kilometer semi_major_axis, double eccentricity, radian inclination, radian LAN, radian w, radian M0)
        : eccentricity(eccentricity),
          semi_major_axis(semi_major_axis),
          inclination(std::fmod(inclination, PI)),
          LAN(std::fmod(LAN, TWOPI)),
          w(w),
          M0(M0),
          v(M0),
          epoch(0),
          reference_body(entt::null) {}

    Orbit(kilometer semi_major_axis, double eccentricity, radian inclination, radian LAN, radian w, radian M0,
          entt::entity reference)
        : eccentricity(eccentricity),
          semi_major_axis(semi_major_axis),
          inclination(std::fmod(inclination, PI)),
          LAN(std::fmod(LAN, TWOPI)),
          w(w),
          M0(M0),
          v(M0),
          epoch(0),
          reference_body(reference) {}

    Orbit(const Orbit& orbit)
        : eccentricity(orbit.eccentricity),
          semi_major_axis(orbit.semi_major_axis),
          inclination(orbit.inclination),
          LAN(orbit.LAN),
          w(orbit.w),
          M0(orbit.M0),
          v(orbit.M0),
          epoch(orbit.epoch),
          GM(orbit.GM),
          reference_body(orbit.reference_body) {}

    double GetMtElliptic(double time) const { return normalize_radian(M0 + (time - epoch) * nu()); }

    double GetOrbitingRadius() const;

    double GetOrbitingRadius(double v) const;

    // Orbital period
    double T() const { return 2 * PI * sqrt(semi_major_axis * semi_major_axis * semi_major_axis / GM); }

    // Mean motion
    double nu() const { return sqrt(GM / abs(semi_major_axis * semi_major_axis * semi_major_axis)); }

    double GetApoapsis() const { return semi_major_axis * (1 + eccentricity); }

    double GetPeriapsis() const { return semi_major_axis * (1 - eccentricity); }

    double TimeToTrueAnomaly(double v2) const;

    double OrbitalVelocity();

    double OrbitalVelocityAtTrueAnomaly(double true_anomaly);

    std::string ToHumanString();
};

inline std::ostream& operator<<(std::ostream& outs, const Orbit& orb) {
    return outs << "(a=" << orb.semi_major_axis << ", e=" << orb.eccentricity << ", i=" << orb.inclination
                << ", LAN=" << orb.LAN << ", w=" << orb.w << ", GM=" << orb.GM << ", v=" << orb.v << ", M0=" << orb.M0
                << ", t=" << orb.epoch << ", ref=" << (uint32_t)orb.reference_body << ")";
}

/**
 * True anomaly of the apoapsis
 */
constexpr double apoapsis = PI;
/**
 * True anomaly of the periapsis
 */
constexpr double periapsis = 0;

struct SetTrueAnomaly {
    double true_anomaly;
};

/// <summary>
/// Transforms a vector to the orbital plane vector
/// </summary>
/// All units are in radians
/// <param name="LAN">Longitude of the ascending node</param>
/// <param name="i">Inclination</param>
/// <param name="w">Argument of periapsis</param>
/// <param name="vec">Vector to convert</param>
glm::dvec3 ConvertOrbParams(const double LAN, const double i, const double w, const glm::dvec3& vec);

glm::dvec3 ConvertToOrbitalVector(const double LAN, const double i, const double w, const double v,
                                  const glm::dvec3& vec);

glm::dvec3 MatrixConvertOrbParams(const double LAN, const double i, const double w, const glm::dvec3& vec);

/// <summary>
/// Updates the orbit's true anomaly.
/// </summary>
/// You need to call it before calculating anything to do with the orbit
/// <param name="orb"></param>
/// <param name="time"></param>
void UpdateOrbit(Orbit& orb, const second& time);

/// <summary>
/// Get anomaly at epoch
/// </summary>
/// <param name="orb"></param>
/// <param name="epoch"></param>
double GetTrueAnomaly(const Orbit& orb, const second& epoch);

/// Calculates the vector velocity of the orbit
///
/// \param[in] E Eccentric anomaly
/// \param[in] r Orbiting radius
/// \param[in] GM G*M of orbiting body
/// \param[in] a Semi major axis
/// \param[in] e eccentricity
glm::dvec3 CalculateVelocity(const double& E, const kilometer& r, const double& GM, const kilometer& a,
                             const double& e);

glm::dvec3 CalculateVelocityElliptic(const double& E, const kilometer& r, const double& GM, const kilometer& a,
                                     const double& e);

glm::dvec3 CalculateVelocityHyperbolic(const double& E, const double& r, const double& GM, const double& a,
                                       const double& e);

/// Get the circular orbiting velocity for the radius
double GetCircularOrbitingVelocity(const double& GM, const double& radius);

/// @brief Calculates v_inf, the true anomaly for the asymtope for a hyperbolic orbit
/// The orbit is bouded within -v_inf < v < v_inf
/// https://orbital-mechanics.space/the-orbit-equation/hyperbolic-trajectories.html#hyperbolic-trajectories-e-1
/// @param orbit
/// @return
double GetTrueAnomalyToAsymptope(const Orbit& orbit);

/// <summary>
/// Converts position and velocity to orbit.
/// Note: you will have to set the reference body after the orbit is returned
/// </summary>
/// <param name="position">Position of the body</param>
/// <param name="velocity">Velocity of the body</param>
/// <param name="GM">G*M of the orbiting body</param>
Orbit Vec3ToOrbit(const glm::dvec3& position, const glm::dvec3& velocity, const double& GM, const double& time);

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
glm::dvec3 OrbitToVec3(const double& a, const double& e, const radian& i, const radian& LAN, const radian& w,
                       const radian& v);

double OrbitVelocity(const double v, const double e, const double a, const double GM);
double OrbitVelocityAtR(const double GM, const double a, const double r);
double AvgOrbitalVelocity(const Orbit& orb);

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb, double v);
glm::dvec3 OrbitVelocityToVec3(const Orbit& orb);

/// <summary>
/// Computes eccentric anomaly for a elliptic or circular orbit (e < 1) in radians given
/// mean anomaly and eccentricity
/// </summary>
/// <param name="mean_anomaly"></param>
/// <param name="ecc"></param>
/// <param name="steps">Number of steps for the Newton-Raphson. More steps means more precision but less speed</param>
/// <returns>Eccentric anomaly (E)goog</returns>
double SolveKeplerElliptic(const double& mean_anomaly, const double& ecc, const int steps = 200);

/// <summary>
/// Computes eccentric anomaly for a hyperbolic or parabolic orbit (e > 1)
/// in radians given mean anomaly and eccentricity
/// </summary>
/// <param name="mean_anomaly"></param>
/// <param name="ecc"></param>
/// <param name="steps">Number of steps for the Newton-Raphson. More steps means more precision but less speed</param>
/// <returns></returns>
double SolveKeplerHyperbolic(const double& mean_anomaly, const double& ecc, const int steps = 200);

/// <summary>
/// Calculates true anomaly from eccentricity and eccentric anomaly
/// </summary>
/// \param[in] ecc The eccentricity of the orbit
/// \param[in] E The eccentric anomaly of the orbit
double EccentricAnomalyToTrueAnomaly(const double& ecc, const double& E);

double HyperbolicAnomalyToTrueAnomaly(const double& ecc, const double& H);

/// <summary>
/// Gets the Mean anomaly from the time
/// </summary>
/// <param name="M0">Mean anomaly at t=0</param>
/// <param name="nu">G*M of orbiting body</param>
/// <param name="time">Current time</param>
/// <returns></returns>
double GetMtElliptic(const double& M0, const double& nu, const double& time, const double& epoch);

/// <summary>
/// Calculate mean anomaly from time for a hyperbolic object
/// </summary>
/// <param name="Mu">G*M of orbiting body</param>
/// <param name="a">Semi-major axis</param>
/// <param name="d+t">Time from periapsis</param>
/// <returns></returns>
double GetMtHyperbolic(const double& M0, const double& nu, const double& time, const double& epoch);

/// <param name="orbit">[in]  Orbit to compute</param>
/// <param name="time">[in]  Current time (seconds)</param>
/// <param name="E_put">[out] Eccentric anomaly</param>
/// <returns>True anomaly in radians</returns>
radian TrueAnomalyElliptic(const Orbit& orbit, const second& time);
radian TrueAnomalyElliptic(const Orbit& orbit, const second& time, double& E_out);

radian TrueAnomalyHyperbolic(const Orbit& orbit, const second& time);

/// <summary>
/// Calculates Eccentric anomaly
/// </summary>
/// <param name="v">True anomaly</param>
/// <param name="e">Eccentricicy</param>
/// <returns></returns>
radian EccentricAnomaly(double v, double e);
radian HyperbolicAnomaly(double v, double e);
/// <summary>
/// Convert orbit to AU coordinates
/// </summary>
/// <param name="orb">Orbit</param>
/// <param name="theta">Theta to compute</param>
/// <returns>Vector 3 in orbit, in AU</returns>
inline Vec3AU toVec3AU(const Orbit& orb, radian theta) {
    glm::dvec3 vec = OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
    return vec / KmInAu;
}

/// <summary>
/// Applies impulse based on the vector impulse
/// For some reason y is prograde, I'm not sure what the others are.
/// TODO(EhWhoAmI): Determine the axis of thrust for each vector
/// I think x is radial, and z is normal.
/// </summary>
/// <param name="orbit"></param>
/// <param name="impulse"></param>
/// <param name="time"></param>
/// <returns></returns>
Orbit ApplyImpulse(const Orbit& orbit, const glm::dvec3& impulse, double time);

/// <summary>
/// Converts orbit to theta
/// </summary>
/// <param name="orb"></param>
/// <param name="theta">True anomaly (radians)</param>
/// <returns></returns>
inline glm::dvec3 toVec3(const Orbit& orb, radian theta) {
    return OrbitToVec3(orb.semi_major_axis, orb.eccentricity, orb.inclination, orb.LAN, orb.w, theta);
}

glm::dvec3 OrbitTimeToVec3(const Orbit& orb, const second time = 0);
glm::dvec3 OrbitTimeToVelocityVec3(const Orbit& orb, const second time = 0);

inline glm::dvec3 toVec3(const Orbit& orb) { return toVec3(orb, orb.v); }
/// <summary>
/// Converts orbit to AU coorrdinates.
/// </summary>
/// <param name="orb">[in] orbit of the object to convert</param>
/// <returns>Vector 3 in orbit, in AU</returns>
inline Vec3AU toVec3AU(const Orbit& orb) { return toVec3AU(orb, orb.v); }

/// <summary>
/// Updates the position of an orbit, in AU.
/// </summary>
/// <param name="kin">[out] the kinematics of the orbit</param>
/// <param name="orb">[in] the orbit of the orbit</param>
inline void UpdatePos(Kinematics& kin, const Orbit& orb) {
    // Calculate time
    kin.position = toVec3AU(orb);
}

double CalculateTransferTime(const Orbit& orb1, const Orbit& orb2);
double CalculateTransferAngle(const Orbit& start_orbit, const Orbit& end_orbit);
double CalculatePhaseAngle(const Orbit& start_orbit, const Orbit& end_orbit, double epoch);

// https://orbital-mechanics.space/the-orbit-equation/hyperbolic-trajectories.html
// True anomaly of the asymptope for a hyperbolic orbit
// The hyperbolic asymtope is defined by -GetHyperbolicAsymtope < v <GetHyperbolicAsymtope
double GetHyperbolicAsymptopeAnomaly(double eccentricity);

double FlightPathAngle(double eccentricity, double v);

glm::dvec3 GetOrbitNormal(const Orbit& orbit);
glm::dvec3 GetRadialVector(const Orbit& orbit);
glm::dvec3 GetRadialVector(const Orbit& orbit, double true_anomaly);
glm::dvec3 InvertOrbitalVector(const double LAN, const double i, const double w, const double v, const glm::dvec3& vec);
double AngleWith(const Orbit& orbit, const Orbit& second_orbit);
double TrueAnomalyFromVector(const Orbit& orbit, const glm::dvec3& vec);

double AscendingTrueAnomaly(const Orbit& start, const Orbit& dest);
double GetEccentricity(double apoapsis, double periapsis);
}  // namespace cqsp::common::components::types
