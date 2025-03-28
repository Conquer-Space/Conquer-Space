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
#include "common/components/orbit.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace cqsp::common::components::types {
double GetOrbitingRadius(const double& e, const radian& a, const radian& v) {
    if (e > 1) {
        return (a * (e * e - 1)) / (1 + e * cos(v));
    } else {
        return (a * (1 - e * e)) / (1 + e * cos(v));
    }
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
    const glm::dvec3 ecc_v = glm::cross(velocity, h) / GM - glm::normalize(position);

    // Eccentricity
    double e = glm::length(ecc_v);

    // Vector pointing towards the ascending node
    const auto n = glm::dvec3(-h.y, h.x, 0);

    // True anomaly
    double v = glm::angle(glm::normalize(ecc_v), glm::normalize(position));  //acos(std::clamp(m, -1., 1.));
    if (glm::dot(position, velocity) < 0) v = TWOPI - v;

    // Inclination
    const double i = std::acos(h.z / glm::length(h));

    double M0 = 0;
    double E = 0;
    if (e < 1) {
        // Eccentric anomaly
        E = EccentricAnomaly(v, e);
        M0 = E - e * sin(E);
    } else {
        const double F = HyperbolicAnomaly(v, e);
        M0 = e * sinh(F) - F;
        E = F;
    }

    double LAN = acos(glm::clamp(n.x / glm::length(n), -1., 1.));
    if (n.y < 0) LAN = TWOPI - LAN;

    double w = glm::angle(glm::normalize(n), glm::normalize(ecc_v));
    if (n == glm::dvec3(0.0, 0.0, 0.0)) {
        // It's equal to the zero vector so LAN = 0
        LAN = 0;
        // Also figure out w
        w = acos(ecc_v.x / glm::length(ecc_v));
    }
    if (ecc_v.z < 0) w = TWOPI - w;

    double velocity_mag = glm::length(velocity);
    double sma = 1 / (2 / glm::length(position) - velocity_mag * velocity_mag / GM);

    assert((e > 1 && sma <= 0) || (e <= 1 && sma >= 0));

    if (i == 0 || i == PI) {
        // Then figure out the values we want
        // Set LAN and w to zero

        LAN = 0;
        // elliptical equatorial
        if (e > 0 && e < 1) {
            w = acos(ecc_v.x / glm::length(ecc_v));
            if (ecc_v.y < 0) {
                w = TWOPI - w;
            }
        }
    } else if (e == 0) {  // Circular inclined
        w = glm::angle(glm::normalize(n), glm::normalize(position));
        if (position.z < 0) w = TWOPI - w;
        v = 0;
        M0 = 0;
    }
    Orbit orb;
    orb.semi_major_axis = sma;
    orb.eccentricity = e;
    orb.LAN = normalize_radian(LAN);
    orb.w = normalize_radian(w);
    orb.inclination = i;
    orb.M0 = M0;
    orb.epoch = time;
    orb.v = normalize_radian(v);
    orb.GM = GM;
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
    double semi_param = a * (1 - e * e);

    //MatrixConvertOrbParams(LAN, i, w, glm::dvec(r * cos(v), r * sin(v), 0);
    return semi_param *
           ConvertToOrbitalVector(LAN, i, w, 0, glm::dvec3(cos(v) / (1 + e * cos(v)), sin(v) / (1 + e * cos(v)), 0));
}

double OrbitVelocity(const double v, const double e, const double a, const double GM) {
    double r = GetOrbitingRadius(e, a, v);
    double sma = a;

    return sqrt(GM * (2 / abs(r) - 1 / sma));
}

double AvgOrbitalVelocity(const Orbit& orb) { return (PI * 2 * orb.semi_major_axis) / orb.T(); }

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb, double v) {
    if (orb.semi_major_axis == 0) {
        return glm::dvec3(0, 0, 0);
    }
    // Return
    double semi_param = orb.semi_major_axis * (1 - orb.eccentricity * orb.eccentricity);

    return sqrt(orb.GM / semi_param) *
           ConvertOrbParams(orb.LAN, orb.inclination, orb.w, glm::dvec3(-sin(v), (orb.eccentricity + cos(v)), 0));
}

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb) { return OrbitVelocityToVec3(orb, orb.v); }

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
    return 2 * atan((sqrt(ecc + 1) * sinh(H / 2)) / (sqrt(ecc - 1) * cosh(H / 2)));
    // Might need to do a random -2pi or something like that, not sure how to handle it
    // I think if the hyperbolic anomaly is out of bounds we subtract pi? Because the difference is a bit whack
}

double GetMtElliptic(const double& M0, const double& nu, const double& time, const double& epoch) {
    // Calculate
    double Mt = M0 + (time - epoch) * nu;
    return normalize_radian(Mt);
}

double GetMtHyperbolic(const double& M0, const double& nu, const double& time, const double& epoch) {
    double Mt = M0 + (time - epoch) * nu;
    return Mt;  //normalize_radian(Mt);
}

radian TrueAnomalyElliptic(const Orbit& orbit, const second& time) {
    double Mt = GetMtElliptic(orbit.M0, orbit.nu(), time, orbit.epoch);
    double E = SolveKeplerElliptic(Mt, orbit.eccentricity);
    return EccentricAnomalyToTrueAnomaly(orbit.eccentricity, E);
}

radian TrueAnomalyElliptic(const Orbit& orbit, const second& time, double& E_out) {
    double Mt = GetMtElliptic(orbit.M0, orbit.nu(), time, orbit.epoch);
    double E = SolveKeplerElliptic(Mt, orbit.eccentricity);
    E_out = E;
    return EccentricAnomalyToTrueAnomaly(orbit.eccentricity, E);
}

double GetCircularOrbitingVelocity(const double& GM, const double& radius) { return sqrt(GM / radius); }

radian TrueAnomalyHyperbolic(const Orbit& orbit, const second& time) {
    double Mt = GetMtHyperbolic(orbit.M0, orbit.nu(), time, orbit.epoch);
    double H = SolveKeplerHyperbolic(Mt, orbit.eccentricity);
    double v = HyperbolicAnomalyToTrueAnomaly(orbit.eccentricity, H);
    assert((-GetHyperbolicAsymptopeAnomaly(orbit.eccentricity) < v &&
            v < GetHyperbolicAsymptopeAnomaly(orbit.eccentricity) &&
            "Orbit needs to be between the hyperbolic asymtopes!"));
    return v;
}

// https://space.stackexchange.com/questions/27602/what-is-hyperbolic-eccentric-anomaly-f
radian EccentricAnomaly(double v, double e) { return 2 * atan(tan(v / 2) * sqrt((1 - e) / (1 + e))); }

radian HyperbolicAnomaly(double v, double e) { return 2 * atanh(tan(v / 2) * sqrt((e - 1) / (e + 1))); }

void UpdateOrbit(Orbit& orb, const second& time) {
    // Get the thingy
    if (orb.eccentricity < 1) {
        orb.v = TrueAnomalyElliptic(orb, time);
    } else {
        orb.v = TrueAnomalyHyperbolic(orb, time);
    }
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

glm::dvec3 CalculateVelocity(const double& E, const kilometer& r, const double& GM, const kilometer& a,
                             const double& e) {
    // Elliptic orbit
    if (e < 1) {
        return CalculateVelocityElliptic(E, r, GM, a, e);
    }
    return CalculateVelocityHyperbolic(E, r, GM, a, e);
}

glm::dvec3 CalculateVelocityHyperbolic(const double& E, const double& r, const double& GM, const double& a,
                                       const double& e) {
    return (double)(sqrt(abs(GM * a)) / r) * glm::dvec3(sinh(E), -sqrt(e * e - 1) * cosh(E), 0);
}

glm::dvec3 CalculateVelocityElliptic(const double& E, const kilometer& r, const double& GM, const kilometer& a,
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

glm::dvec3 OrbitTimeToVec3(const Orbit& orb, const second time) {
    double v = 0;
    if (orb.eccentricity < 1) {
        v = TrueAnomalyElliptic(orb, time);
    } else {
        v = TrueAnomalyHyperbolic(orb, time);
    }
    return toVec3(orb, v);
}

glm::dvec3 OrbitTimeToVelocityVec3(const Orbit& orb, const second time) {
    double v = 0;
    if (orb.eccentricity < 1) {
        v = TrueAnomalyElliptic(orb, time);
    } else {
        v = TrueAnomalyHyperbolic(orb, time);
    }
    return OrbitVelocityToVec3(orb, v);
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
    double phase_angle = PI - t_trans * end_orbit.nu();
    return phase_angle;
}

double CalculatePhaseAngle(const Orbit& start_orbit, const Orbit& end_orbit, double epoch) {
    double a = GetTrueAnomaly(start_orbit, epoch) + start_orbit.w;
    double b = GetTrueAnomaly(end_orbit, epoch) + end_orbit.w;

    return b - a;
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
double Orbit::TimeToTrueAnomaly(double v2) const {
    // If it's a hyperbolic orbit, we will have to use different equations.
    // The mean anomaly will be positive, so
    // Get eccentric anomaly
    // Assume current v is v0.
    if (eccentricity > 1) {
        // Use hyperbolic equations
        const double F0 = std::acosh((eccentricity + cos(v)) / (1 + eccentricity * cos(v)));
        double M0 = eccentricity * sinh(F0) - F0;

        const double F = std::acosh((eccentricity + cos(v2)) / (1 + eccentricity * cos(v2)));
        double M = eccentricity * sinh(F) - F;
        double t = (M0 - M) / nu();
        return t;
    } else {
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
        double t = (M - M0) / nu();
        if (t < 0) {
            t = T() + t;
        }
        return (t);
    }
}
}  // namespace cqsp::common::components::types
