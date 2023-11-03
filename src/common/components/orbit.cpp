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

namespace cqsp::common::components::types {
double GetOrbitingRadius(const double& e, const double& a, const double& v) {
    // Calculate the math
    return (a * (1 - e * e)) / (1 + e * cos(v));
}

glm::dvec3 ConvertOrbParams(const double LAN, const double i, const double w, const glm::dvec3& vec) {
    return glm::dquat {glm::dvec3(0, 0, LAN)} * glm::dquat {glm::dvec3(i, 0, 0)} * glm::dquat {glm::dvec3(0, 0, w)} *
           vec;
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

    double w = acos(glm::dot(n, ecc_v) / (e * glm::length(n)));
    if (ecc_v.z < 0) w = TWOPI - w;
    if (glm::length(n) == 0) w = 0;

    double velocity_mag = glm::length(velocity);
    double sma = 1 / (2 / glm::length(position) - velocity_mag * velocity_mag / GM);

    Orbit orb;
    orb.semi_major_axis = sma;
    orb.eccentricity = e;
    orb.LAN = LAN;
    orb.w = w;
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
        return glm::vec3(0, 0, 0);
    }
    double r = GetOrbitingRadius(e, a, v);
    return ConvertOrbParams(LAN, i, w, glm::vec3(r * cos(v), r * sin(v), 0));
}

double AvgOrbitalVelocity(const Orbit& orb) { return (PI * 2 * orb.semi_major_axis) / orb.T; }

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb, double v) {
    if (orb.semi_major_axis == 0) {
        return glm::vec3(0, 0, 0);
    }
    double r = GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, v);
    glm::vec3 velocity = CalculateVelocity(orb.E, r, orb.GM, orb.semi_major_axis, orb.eccentricity);
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

    while ((it < steps) && (abs(de) > 1.0E-5)) {  // normal accuracy is 1.0e-10
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

    while ((it < steps) && (abs(de) > 1.0E-5)) {  // normal accuracy is 1.0e-10
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

glm::vec3 CalculateVelocity(const double& E, const double& r, const double& GM, const double& a, const double& e) {
    // Elliptic orbit
    if (e < 1) {
        return CalculateVelocityElliptic(E, r, GM, a, e);
    }
    return CalculateVelocityHyperbolic(E, r, GM, a, e);
}

glm::vec3 CalculateVelocityHyperbolic(const double& E, const double& r, const double& GM, const double& a,
                                      const double& e) {
    return (float)(sqrt(-GM * a) / r) * glm::vec3(sinh(E), -sqrt(e * e - 1) * cosh(E), 0);
}

glm::vec3 CalculateVelocityElliptic(const double& E, const double& r, const double& GM, const double& a,
                                    const double& e) {
    return ((float)(sqrt(GM * a) / r) * glm::vec3(-sin(E), sqrt(1 - e * e) * cos(E), 0));
}

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
double CalculateTransferAngle(const Orbit& orb1, const Orbit& orb2) {
    double transfer_time = CalculateTransferTime(orb1, orb2);
    return (orb2.v - orb1.v) - transfer_time * orb2.nu;
}
double GetEccentricAnomaly(double eccentricity, double theta) {
    //return std::acos()
    return 0;
}

double GetHyperbolicAsymptopeAnomaly(double eccentricity) { return std::acos(-1 / eccentricity); }
}  // namespace cqsp::common::components::types
