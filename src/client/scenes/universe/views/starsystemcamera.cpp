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

namespace cqsp::client::systems {
StarSystemCamera::StarSystemCamera() : scroll(5), view_x(0), view_y(0), view_center(glm::vec3(1, 1, 1)) {}

void StarSystemCamera::CalculateCameraMatrix(int window_width, int window_height, float delta_time) {
    cam_pos = CameraPositionNormalized() * static_cast<float>(scroll);
    camera_time += delta_time;
    cam_up = glm::mix(cam_up, target_cam_up, (glm::clamp(camera_time / max_camera_time, 0.f, 1.f)));
    camera_matrix = glm::lookAt(cam_pos, glm::vec3(0.f, 0.f, 0.f), cam_up);
    projection = glm::infinitePerspective(glm::radians(45.f),
                                          static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f);
    viewport = glm::vec4(0.f, 0.f, static_cast<float>(window_width), static_cast<float>(window_height));
}

glm::vec3 StarSystemCamera::CameraPositionNormalized() {
    return glm::vec3(std::cos(view_y) * std::sin(view_x), std::cos(view_y) * std::cos(view_x), std::sin(view_y));
}

void StarSystemCamera::SetCameraUp(const glm::vec3 _target_cam_up) {
    initial_cam_up = cam_up;
    target_cam_up = _target_cam_up;
    camera_time = 0.f;
}

void StarSystemCamera::ResetCameraUp() {
    initial_cam_up = cam_up;
    target_cam_up = default_cam_up;
    camera_time = 0.f;
}

float StarSystemCamera::Tween(float t) {
    if (t < 0.5f) {
        return TweenFunction(2.0 * t) * 0.5f;
    } else {
        return 0.5f + TweenOut(2.0 * t - 1.0f) * 0.5f;
    }
}

float StarSystemCamera::TweenOut(float t) { return 1.f - TweenFunction(1.0f - t); }

float StarSystemCamera::TweenFunction(float t) { return t * t * t * t * t; }

void StarSystemCamera::FixCameraUp(const glm::vec3 _target_cam_up) { target_cam_up = initial_cam_up = _target_cam_up; }
}  // namespace cqsp::client::systems
