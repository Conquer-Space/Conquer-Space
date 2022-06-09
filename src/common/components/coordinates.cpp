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
#include "common/components/coordinates.h"

#include "common/components/units.h"
#include <iostream>
#include <iomanip>

namespace cqsp::common::components::types {
glm::dvec3 ConvertOrbParams(const double LAN, const double i, const double w,
                           const glm::dvec3& vec) {
    return glm::dquat{glm::dvec3(0, 0, -LAN)} * glm::dquat{glm::dvec3(-i, 0, 0)} *
           glm::dquat{glm::dvec3(0, 0, -w)} * vec;
}

double GetOrbitingRadius(const double& e, const double& a, const double& v) {
    // Calculate the math
    return (a * (1 - e * e)) / (1 + e * cos(v));
}

// https://downloads.rene-schwarz.com/download/M002-Cartesian_State_Vectors_to_Keplerian_Orbit_Elements.pdf
Orbit Vec3ToOrbit(const glm::dvec3& position, const glm::dvec3& velocity,
                 const double& Mu, const double& time) {
    // Orbital momentum vector
    const auto h = glm::cross(position, velocity);
    // Eccentricity vector
    const auto ecc_v = glm::cross(velocity, h) / Mu - glm::normalize(position);

    // Eccentricity
    double e = glm::length(ecc_v);

    // Vector pointing towards the ascending node
    const auto n = glm::dvec3(-h.y, h.x, 0);
    // True anomaly
    double m = glm::dot(ecc_v, position) / (e * glm::length(position));
    double v = acos(std::clamp(m, -1., 1.));
    if (glm::dot(position, velocity) < 0) v = PI * 2 - v;
    if (m >= 1) v = 0;

    // Inclination
    const double i = std::acos(h.z / glm::length(h));

    // Eccentric anomaly
    const double E = 2 * atan(tan(v / 2) / sqrt((1 + e) / (1 - e)));
    double M0 = E - e * sin(E);
    double LAN = (n.x / glm::length(n));
    if (n.y < 0) LAN = PI * 2 - LAN;
    if (glm::length(n) == 0) LAN = 0;

    double w = acos(glm::dot(n, ecc_v) / (e * glm::length(n)));
    if (ecc_v.z < 0) w = PI * 2 - w;
    if (glm::length(n) == 0) w = 0;

    double velocity_mag = glm::length(velocity);
    double sma = 1 / (2 / glm::length(position) - velocity_mag * velocity_mag / Mu);

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
    orb.Mu = Mu;
    return orb;
}

// https://downloads.rene-schwarz.com/download/M001-Keplerian_Orbit_Elements_to_Cartesian_State_Vectors.pdf
glm::dvec3 OrbitToVec3(const double& a, const double& e, const radian& i,
                       const radian& LAN, const radian& w, const radian& v) {
    if (a == 0) {
        return glm::vec3(0, 0, 0);
    }
    double r = GetOrbitingRadius(e, a, v);
    return ConvertOrbParams(LAN, i, w, glm::vec3(r * cos(v), r * sin(v), 0));
}

double AvgOrbitalVelocity(const Orbit& orb) { return (PI * 2 * orb.semi_major_axis) / orb.T; }

glm::dvec3 OrbitVelocityToVec3(const Orbit& orb, double v) {
    // Convert the values
    if (orb.semi_major_axis == 0) {
        return glm::vec3(0, 0, 0);
    }
    double r = (orb.semi_major_axis * (1 - orb.eccentricity * orb.eccentricity)) /
                    (1 + orb.eccentricity * cos(v));
    return CalculateVelocity(orb.E, r, orb.Mu, orb.semi_major_axis, orb.eccentricity);
}

double SolveKepler(const double& mean_anomaly, const double& ecc, const int steps) {
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

    while ((it < maxit) && (abs(de) > 1.0E-5)) {  // normal accuracy is 1.0e-10
        double new_m = ea - ecc * sin(ea);
        de = (old_m - new_m) / (1.0 - ecc * cos(ea));
        ea = ea + de;
        it = it + 1;
    }
    return ea;
}

double CalculateTrueAnomaly(const double& ecc, const double& E) {
    return 2 * atan2(sqrt(1 + ecc) * sin(E / 2), sqrt(1 - ecc) * cos(E / 2));
}

double GetMt(const double& M0, const double& nu, const double& time, const double &epoch) {
    // Calculate
    double Mt = M0 + (time - epoch) * nu;
    return normalize_radian(Mt);
}

radian TrueAnomaly(const Orbit& orbit, const second& time) {
    double Mt = GetMt(orbit.M0, time, orbit.nu, orbit.epoch);
    double E = SolveKepler(Mt, orbit.eccentricity);
    return CalculateTrueAnomaly(orbit.eccentricity, E);
}

void UpdateOrbit(Orbit& orb, const second& time) {
    double Mt = GetMt(orb.M0, time, orb.nu, orb.epoch);
    double E = SolveKepler(Mt, orb.eccentricity);
    orb.v = CalculateTrueAnomaly(orb.eccentricity, E);
    orb.E = E;
}

glm::vec3 CalculateVelocity(const double& E, const double& r,
                            const double& Mu, const double& a,
                            const double& e) {
    return ((float)(sqrt(Mu * a) / r) *
            glm::vec3(-sin(E), sqrt(1 - e * e) * cos(E), 0));
}

glm::vec3 toVec3(const SurfaceCoordinate& coord, const float& radius) {
    return glm::vec3(cos(coord.r_latitude()) * sin(coord.r_longitude()),
                     sin(coord.r_latitude()),
                     cos(coord.r_latitude()) * cos(coord.r_longitude())) *
           radius;
}
}  // namespace qsp::common::components::types
