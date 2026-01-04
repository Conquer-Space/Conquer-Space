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

#include <glm/glm.hpp>

#include "core/universe.h"
#include "engine/application.h"

namespace cqsp::client::systems {
/**
 * Holds camera state.
 */
struct StarSystemCamera {
 public:
    StarSystemCamera();
    void CalculateCameraMatrix(int window_width, int window_height, float delta_time);
    glm::vec3 CameraPositionNormalized();

    glm::vec3 cam_pos;
    const glm::vec3 default_cam_up = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cam_up = default_cam_up;
    glm::mat4 camera_matrix;
    glm::mat4 projection;
    glm::vec4 viewport;
    glm::vec3 view_center;
    double scroll;

    // The angle the camera is looking from
    float view_x = 0;
    // The angle the camera is looking away from
    float view_y = 0;

    glm::vec3 target_cam_up = default_cam_up;
    glm::vec3 initial_cam_up;

    const float max_camera_time = 2.f;
    float camera_time = 0.f;

    void SetCameraUp(const glm::vec3 _target_cam_up);
    void FixCameraUp(const glm::vec3 _target_cam_up);
    void ResetCameraUp();

    bool CameraUpDone() { return camera_time > max_camera_time; }

 private:
    // Custom animation function to go from 0 to 1.
    float Tween(float t);
    float TweenOut(float t);
    float TweenFunction(float t);
};
}  // namespace cqsp::client::systems
