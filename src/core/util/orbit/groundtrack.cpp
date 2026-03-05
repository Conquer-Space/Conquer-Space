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
#include "core/util/orbit/groundtrack.h"

namespace cqsp::core::util {
glm::quat GetBodyRotation(double time, const components::bodies::Body& body) {
    float rot =
        static_cast<float>(components::bodies::GetPlanetRotationAngle(time, body.rotation, body.rotation_offset));
    if (body.rotation == 0) {
        rot = 0;
    }
    return glm::quat {{(float)-body.axial, 0, 0}} *
           glm::quat {{0, 0, (float)std::fmod(rot, core::components::types::TWOPI)}};
}

components::types::SurfaceCoordinate GetGroundTrack(const glm::quat& planet_rotation_matrix,
                                                    const glm::vec3& position) {
    glm::vec3 p = glm::normalize(position);
    return components::types::ToSurfaceCoordinate(glm::inverse(planet_rotation_matrix) * p);
}
}  // namespace cqsp::core::util
