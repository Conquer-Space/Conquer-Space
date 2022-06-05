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

namespace cqsp::common::components::types {
glm::vec3 ConvertOrbParams(const double LAN, const double i, const double w,
                           const glm::vec3& vec) {
    return glm::quat{glm::vec3(0, 0, -LAN)} * glm::quat{glm::vec3(-i, 0, 0)} *
           glm::quat{glm::vec3(0, 0, -w)} * vec;
}
glm::vec3 OrbitToVec3(const double& a, const double& e, const radian& i,
                       const radian& LAN, const radian& w, const radian& v) {
    if (a == 0) {
        return glm::vec3(0, 0, 0);
    }
    double r = (a * (1 - e * e)) / (1 + e * cos(v));
    glm::vec3 vec =
        ConvertOrbParams(-LAN, -i, -w, glm::vec3(r * cos(v), r * sin(v), 0));
    return glm::vec3(vec.x, vec.z, vec.y);
}

glm::vec3 OrbitVelocityToVec3(const Orbit& orb, double v) {
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

double CalculateTrueAnomaly(const double& ecc,
                                                             const double& E) {
    return 2 * atan2(sqrt(1 + ecc) * sin(E / 2), sqrt(1 - ecc) * cos(E / 2));
}

double GetMt(const double& M0, const double& nu, const double& time) {
    // Calculate
    double Mt = M0 + time * nu;
    return normalize_radian(Mt);
}

radian TrueAnomaly(const Orbit& orbit, const second& time) {
    double Mt = GetMt(orbit.M0, time, orbit.nu);
    double E = SolveKepler(Mt, orbit.eccentricity);
    return CalculateTrueAnomaly(orbit.eccentricity, E);
}

void UpdateOrbit(Orbit& orb, const second& time) {
    double Mt = GetMt(orb.M0, time, orb.nu);
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
