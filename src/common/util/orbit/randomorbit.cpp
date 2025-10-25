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
#include "common/util/orbit/randomorbit.h"

namespace cqsp::common::util {
common::components::types::Orbit OrbitGenerator::GenerateOrbit(double GM, double time) {
    auto pair = GenerateVectors();
    return components::types::Vec3ToOrbit(pair.first, pair.second, GM, time);
}

/**
 * Generates a position and velocity pair.
 */
std::pair<glm::dvec3, glm::dvec3> OrbitGenerator::GenerateVectors() {
    return std::make_pair(RandomVector(position_distribution), RandomVector(velocity_distribution));
}

double OrbitGenerator::RandomValue(double range) {
    std::uniform_real_distribution<> dist(-range, range);
    return dist(gen);
}

glm::dvec3 OrbitGenerator::RandomVector(const glm::dvec3 &range) {
    return glm::dvec3(RandomValue(range.x), RandomValue(range.y), RandomValue(range.z));
}
}  // namespace cqsp::common::util
