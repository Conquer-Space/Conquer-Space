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
#include "client/scenes/universe/views/starsystemcamera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "starsystemcamera.h"

namespace cqsp::client::systems {
StarSystemCamera::StarSystemCamera() : scroll(5), view_x(0), view_y(0), view_center(glm::vec3(1, 1, 1)) {}

void StarSystemCamera::CalculateCameraMatrix(int window_width, int window_height) {
    cam_pos = glm::vec3(cos(view_y) * sin(view_x), sin(view_y), cos(view_y) * cos(view_x)) * (float)scroll;
    cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_matrix = glm::lookAt(cam_pos, glm::vec3(0.f, 0.f, 0.f), cam_up);
    projection = glm::infinitePerspective(glm::radians(45.f),
                                          static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f);
    viewport = glm::vec4(0.f, 0.f, static_cast<float>(window_width), static_cast<float>(window_height));
}
}  // namespace cqsp::client::systems
