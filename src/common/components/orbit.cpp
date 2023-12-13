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
#include "common/components/orbit.h"

#include <algorithm>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace cqsp::common::components::types {
double GetOrbitingRadius(const double& e, const double& a, const double& v) {
    return (a * (1 - e * e)) / (1 + e * cos(v));
}

glm::dvec3 MatrixConvertOrbParams(const double LAN, const double i, const double w, const glm::dvec3& vec) {
    return glm::dvec3(vec.x * (cos(w) * cos(LAN) - sin(w) * cos(i) * sin(LAN)) -
                          vec.y * (sin(w) * cos(LAN) + cos(w) * cos(i) * sin(LAN)),
                      vec.x * (cos(w) * sin(LAN) + sin(w) * cos(i) * sin(LAN)) +
                          vec.y * (cos(w) * cos(i) * cos(LAN) - sin(w) * sin(LAN)),
                      vec.x * (sin(LAN) * sin(i)) + vec.y * (cos(w) * sin(i)));
}

glm::dvec3 ConvertOrbParams(const double LAN, const double i, const double w, const glm::dvec3& vec) {
    return glm::dquat {glm::dvec3(0, 0, LAN)} * glm::dquat {glm::dvec3(i, 0, 0)} * glm::dquat {glm::dvec3(0, 0, w)} *
           vec;
}

glm::dvec3 ConvertToOrbitalVector(const double LAN, const double i, const double w, const double v,
                                  const glm::dvec3& vec) {
    return glm::dquat {glm::dvec3(0, 0, LAN)} * glm::dquat {glm::dvec3(i, 0, 0)} * glm::dquat {glm::dvec3(0, 0, w)} *
           glm::dquat {glm::dvec3(0, 0, v)} * vec;
}

// https://downloads.rene-schwarz.com/download/M002-Cartesian_State_Vectors_to_Keplerian_Orbit_Elements.pdf
Orbit Vec3ToOrbit(const glm::dvec3& position, const glm::dvec3& velocity, const double& GM, const double& time) {
    // Orbital momentum vector
    const auto h = glm::cross(position, velocity);
    // Eccentricity vector
    const auto ecc_v = glm::cross(velocity, h) / GM - glm::normalize(position);

    // Eccentricity
    double e = glm::length(ecc_v);

    // TODO(EhWhoAmI): if e > 1, then it's a hyperbolic orbit
    // Vector pointing towards the ascending node
    const auto n = glm::dvec3(-h.y, h.x, 0);
    // True anomaly
    double m = glm::dot(ecc_v, position) / (e * glm::length(position));
    double v = acos(std::clamp(m, -1., 1.));
    if (glm::dot(position, velocity) < 0) v = TWOPI - v;
    if (m >= 1) v = 0;

    // Inclination
    const double i = std::acos(h.z / glm::length(h));
    // Hyperbolic eccentric anomaly

    double T = n.x / glm::length(n);
    double M0 = 0;
    double E = 0;
    if (e < 1) {
        // Eccentric anomaly
        E = 2 * atan(tan(v / 2) / sqrt((1 + e) / (1 - e)));
        M0 = E - e * sin(E);
    } else {
        const double F = 2 * atanh(sqrt((e - 1) / (e + 1)) * tan(v / 2));
        M0 = e * sinh(F) - F;
        E = F;
    }
    double LAN = acos(glm::clamp(T, -1., 1.));
    if (n.y < 0) LAN = TWOPI - LAN;
    if (glm::length(n) == 0) LAN = 0;

    double w = acos(std::clamp(glm::dot(n, ecc_v) / (e * glm::length(n)), -1., 1.));
    if (ecc_v.z < 0) w = TWOPI - w;
    if (e == 0) w = 0;
    if (glm::length(n) == 0) w = 0;

    double velocity_mag = glm::length(velocity);
    double sma = 1 / (2 / glm::length(position) - velocity_mag * velocity_mag / GM);

    Orbit orb;
    orb.semi_major_axis = sma;
    orb.eccentricity = e;
    orb.LAN = normalize_radian(LAN);
    orb.w = normalize_radian(w);
    orb.inclination = i;
    orb.M0 = M0;
    orb.epoch = time;
    orb.v = v;
    orb.E = E;
    orb.GM = GM;
    orb.CalculateVariables();
    return orb;
}

double GetTrueAnomalyToAsymptope(const Orbit& orbit) { return std::acos(-1 / orbit.eccentricity); }

// https://downloads.rene-schwarz.com/download/M001-Keplerian_Orbit_Elements_to_Cartesian_State_Vectors.pdf
glm::dvec3 OrbitToVec3(const double& a, const double& e, const radian& i, const radian& LAN, const radian& w,
                       const radian& v) {
    if (a == 0) {
        return glm::dvec3(0, 0, 0);
    }
    double r = GetOrbitingRadius(e, a, v);
    //MatrixConvertOrbParams(LAN, i, w, glm::dvec(r * cos(v), r * sin(v), 0);
    return r * ConvertToOrbitalVector(LAN, i, w, v, glm::vec3(1, 0, 0));
}

double OrbitVelocity(const double v, const double e, const double a, const double GM) {
    double r = GetOrbitingRadius(e, a, v);
    return sqrt(GM * (2 / r - 1 / a));
}

double AvgOrbitalVelocity(const Orbit& orb) { return (PI * 2 * orb.semi_major_axis) / orb.T; }

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb, double v) {
    if (orb.semi_major_axis == 0) {
        return glm::dvec3(0, 0, 0);
    }
    double E = 2 * atan(tan(v / 2) / sqrt((1 + orb.eccentricity) /
                                          (1 - orb.eccentricity)));  //SolveKeplerElliptic(v, orb.eccentricity);
    double r = GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, v);
    glm::dvec3 velocity = CalculateVelocity(E, r, orb.GM, orb.semi_major_axis, orb.eccentricity);
    return ConvertOrbParams(orb.LAN, orb.inclination, orb.w, velocity);
}

double SolveKeplerElliptic(const double& mean_anomaly, const double& ecc, const int steps) {
    if (abs(ecc) < 1.0E-9) {
        return mean_anomaly;
    }
    int it = 0;

    double de = 1000.0;

    double ea = mean_anomaly;
    double old_m = mean_anomaly;

    while ((it < steps) && (abs(de) > 1.0E-10)) {
        double new_m = ea - ecc * sin(ea);
        de = (old_m - new_m) / (1.0 - ecc * cos(ea));
        ea += de;
        it++;
    }
    return ea;
}

double SolveKeplerHyperbolic(const double& mean_anomaly, const double& ecc, const int steps) {
    if (abs(ecc) < 1.0E-9) {
        return mean_anomaly;
    }
    int it = 0;

    double de = 1000.0;

    double ea = mean_anomaly;
    double old_m = mean_anomaly;

    while ((it < steps) && (abs(de) > 1.0E-10)) {
        double new_m = ecc * sinh(ea) - ea;
        de = (old_m - new_m) / (ecc * cosh(ea) - 1.0);
        ea += de;
        it++;
    }
    return ea;
}

double EccentricAnomalyToTrueAnomaly(const double& ecc, const double& E) {
    return 2 * atan2(sqrt(1 + ecc) * sin(E / 2), sqrt(1 - ecc) * cos(E / 2));
}

double HyperbolicAnomalyToTrueAnomaly(const double& ecc, const double& H) {
    return 2 * atan(sqrt((ecc + 1.) / (ecc - 1.)) * tanh(H / 2));
}

double GetMtElliptic(const double& M0, const double& nu, const double& time, const double& epoch) {
    // Calculate
    double Mt = M0 + (time - epoch) * nu;
    return normalize_radian(Mt);
}

double GetMtHyperbolic(const double& Mu, const double& a, const double& d_t) { return sqrt(Mu / (-a * a * a)) * d_t; }

radian TrueAnomalyElliptic(const Orbit& orbit, const second& time) {
    double Mt = GetMtElliptic(orbit.M0, orbit.nu, time, orbit.epoch);
    double E = SolveKeplerElliptic(Mt, orbit.eccentricity);
    return EccentricAnomalyToTrueAnomaly(orbit.eccentricity, E);
}

radian TrueAnomalyElliptic(const Orbit& orbit, const second& time, double& E_out) {
    double Mt = GetMtElliptic(orbit.M0, orbit.nu, time, orbit.epoch);
    double E = SolveKeplerElliptic(Mt, orbit.eccentricity);
    E_out = E;
    return EccentricAnomalyToTrueAnomaly(orbit.eccentricity, E);
}

double GetCircularOrbitingVelocity(const double& GM, const double& radius) { return sqrt(GM / radius); }

radian TrueAnomalyHyperbolic(const Orbit& orbit, const second& time) {
    // Get the time
    double Mt = GetMtHyperbolic(orbit.GM, orbit.semi_major_axis, time - orbit.epoch);
    double H = SolveKeplerHyperbolic(Mt, orbit.eccentricity);
    return HyperbolicAnomalyToTrueAnomaly(orbit.eccentricity, H);
}

void UpdateOrbit(Orbit& orb, const second& time) {
    // Get the thingy
    double E = 0;
    if (orb.eccentricity < 1) {
        orb.v = TrueAnomalyElliptic(orb, time, E);
    } else {
        orb.v = TrueAnomalyHyperbolic(orb, time);
    }
    orb.E = E;
}

double GetTrueAnomaly(const Orbit& orb, const second& epoch) {
    double v = 0;
    if (orb.eccentricity < 1) {
        v = TrueAnomalyElliptic(orb, epoch);
    } else {
        v = TrueAnomalyHyperbolic(orb, epoch);
    }
    return v;
}

glm::dvec3 CalculateVelocity(const double& E, const double& r, const double& GM, const double& a, const double& e) {
    // Elliptic orbit
    if (e < 1) {
        return CalculateVelocityElliptic(E, r, GM, a, e);
    }
    return CalculateVelocityHyperbolic(E, r, GM, a, e);
}

glm::dvec3 CalculateVelocityHyperbolic(const double& E, const double& r, const double& GM, const double& a,
                                       const double& e) {
    return (double)(sqrt(-GM * a) / r) * glm::dvec3(sinh(E), -sqrt(e * e - 1) * cosh(E), 0);
}

glm::dvec3 CalculateVelocityElliptic(const double& E, const double& r, const double& GM, const double& a,
                                     const double& e) {
    return ((sqrt(GM * a) / r) * glm::dvec3(-sin(E), sqrt(1 - e * e) * cos(E), 0));
}

Orbit ApplyImpulse(const Orbit& orbit, const glm::dvec3& impulse, double time) {
    // Calculate v at epoch
    // Move the orbit
    const double v = GetTrueAnomaly(orbit, time);
    const glm::dvec3& norm_impulse = ConvertToOrbitalVector(orbit.LAN, orbit.inclination, orbit.w, v, impulse);
    const glm::dvec3 position = toVec3(orbit, v);
    const glm::dvec3 velocity = OrbitVelocityToVec3(orbit, v);

    // Rotate the vector based off the velocity vector
    // Do quaternions?
    Orbit new_orbit = Vec3ToOrbit(position, velocity + norm_impulse, orbit.GM, time);
    new_orbit.reference_body = orbit.reference_body;
    return new_orbit;
}

double OrbitVelocityAtR(const double GM, const double a, const double r) { return sqrt(GM * (2 / r - 1 / a)); }

glm::dvec3 OrbitTimeToVec3(const Orbit& orb, const second& time) {
    double v = 0;
    double E = 0;
    if (orb.eccentricity < 1) {
        v = TrueAnomalyElliptic(orb, time, E);
    } else {
        v = TrueAnomalyHyperbolic(orb, time);
    }
    return toVec3(orb, v);
}

double CalculateTransferTime(const Orbit& orb1, const Orbit& orb2) {
    kilometer transfer_sma = (orb1.semi_major_axis + orb2.semi_major_axis) / 2;
    double e = 1 - orb1.semi_major_axis / transfer_sma;  // Assume it's circular for now
    double v = acos(((transfer_sma * (1 - e * e)) / orb2.semi_major_axis - 1) / e);
    double E = acos((e + cos(v)) / (1 + e * cos(v)));  // Eccentric anomaly
    double TOF = (E - e * sin(E)) * sqrt(transfer_sma * transfer_sma * transfer_sma / orb1.GM);
    return TOF;
}
double CalculateTransferAngle(const Orbit& start_orbit, const Orbit& end_orbit) {
    double new_sma = (start_orbit.semi_major_axis + end_orbit.semi_major_axis) / 2;
    double t_trans = PI * sqrt((new_sma * new_sma * new_sma) / start_orbit.GM);
    // So calculate the phase angle that we have to do
    // Get mean motion of both bodies
    double phase_angle = PI - t_trans * end_orbit.nu;
    return phase_angle;
}

double GetHyperbolicAsymptopeAnomaly(double eccentricity) { return std::acos(-1 / eccentricity); }

double FlightPathAngle(double eccentricity, double v) {
    return atan2(eccentricity * sin(v), 1 - eccentricity * cos(v));
}

glm::dvec3 GetOrbitNormal(const Orbit& orbit) {
    return glm::dquat {glm::dvec3(0, 0, orbit.LAN)} * glm::dquat {glm::dvec3(orbit.inclination, 0, 0)} *
           glm::dquat {glm::dvec3(0, 0, orbit.w)} * glm::dvec3(0, 0, 1);
}

double TrueAnomalyFromVector(const Orbit& orbit, const glm::dvec3& vec) {
    // Get the intersection of this orbit?
    //auto projected = glm::proj(vec, GetOrbitNormal(orbit));
    auto periapsis = toVec3(orbit, 0);
    double angle = glm::angle(glm::normalize(vec), glm::normalize(periapsis));
    // Shamelessly stolen from mechjeb
    // If the vector points to the infalling part of the orbit then we need to do 2 pi minus the
    // angle from Pe to get the true anomaly. Test this by taking the the cross product of the
    // orbit normal and vector to the periapsis. This gives a vector that points to center of the
    // outgoing side of the orbit. If vectorToAN is more than 90 degrees from this vector, it occurs
    // during the infalling part of the orbit.
    if (abs(glm::angle(vec, glm::cross(GetOrbitNormal(orbit), periapsis))) < PI / 2) {
        return angle;
    }
    return PI * 2 - angle;
}

double AscendingTrueAnomaly(const Orbit& start, const Orbit& dest) {
    return normalize_radian(TrueAnomalyFromVector(start, glm::cross(GetOrbitNormal(start), GetOrbitNormal(dest))));
}

// https://space.stackexchange.com/questions/54396/how-to-calculate-the-time-to-reach-a-given-true-anomaly
double Orbit::TimeToMeanAnomaly(double v2) const {
    // If it's a hyperbolic orbit, we will have to use different equations.
    // The mean anomaly will be positive, so
    // Get eccentric anomaly
    // Assume current v is v0.
    const double E0 = std::acos((eccentricity + cos(v)) / (1 + eccentricity * cos(v)));
    double M0 = E0 - std::sin(E0) * eccentricity;
    if (v > PI) {
        M0 *= -1;
    }

    const double E = std::acos((eccentricity + cos(v2)) / (1 + eccentricity * cos(v2)));
    double M = E - std::sin(E) * eccentricity;

    if (v2 > PI) {
        M *= -1;
    }
    double t = (M - M0) / nu;
    if (t < 0) {
        t = T + t;
    }
    return (t);
}
}  // namespace cqsp::common::components::types
