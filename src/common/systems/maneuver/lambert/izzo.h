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

#include <vector>

#include <glm/vec3.hpp>

namespace cqsp::common::systems::lambert {
class Izzo {
 public:
    // https://github.com/esa/pykep/blob/8b0e9444d09b909d7d1d11e951c8efcfde0a2ffd/src/lambert_problem.cpp
    Izzo(const glm::dvec3& r1, const glm::dvec3& r2, double tof, double mu, int cw, int revs);
    glm::dvec3 Solve(const glm::dvec3& v_start);

 private:
    void FindXY(double lambda, double T);
    void dTdx(double& DT, double& DDT, double DDDT, const double x, const double T);
    int householder(double& x0, const double T, const int N, const double eps, const int iter_max);
    double x2tof2(const double x, const int N);
    double x2tof(const double x, const int N);
    double hypergeometricF(double z, double tol);

    const glm::dvec3 r1;
    const glm::dvec3 r2;
    const double tof;
    const double mu;
    const int cw;
    const int revs;

    std::vector<glm::dvec3> v1;
    std::vector<glm::dvec3> v2;
    std::vector<int> iters;
    std::vector<double> x;
    double lambda;
    double lambda2;
    double lambda3;

    double DT = 0.0;
    double DDT = 0.0;
    double DDDT = 0.0;
};
}  // namespace cqsp::common::systems::lambert
