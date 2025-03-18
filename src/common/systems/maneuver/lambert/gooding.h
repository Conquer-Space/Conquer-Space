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
class Gooding {
 public:
    // https://github.com/esa/pykep/blob/8b0e9444d09b909d7d1d11e951c8efcfde0a2ffd/src/lambert_problem.cpp
    Gooding(const glm::dvec3& r1, const glm::dvec3& r2, double tof, double mu, int revs)
        : r1(r1), r2(r2), tof(tof), mu(mu), revs(revs) {}
    glm::dvec3 Solve();

 private:
    const glm::dvec3 r1;
    const glm::dvec3 r2;
    double tof;
    double mu;
    int revs;
};
}  // namespace cqsp::common::systems::lambert
