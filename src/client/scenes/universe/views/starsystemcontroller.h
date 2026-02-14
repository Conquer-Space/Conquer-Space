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

#include <optional>

#include <glm/glm.hpp>

#include "client/scenes/universe/views/starsystemcamera.h"
#include "core/components/coordinates.h"
#include "core/universe.h"
#include "engine/application.h"

namespace cqsp::client::systems {
class StarSystemViewUI;
class SysStarSystemRenderer;
class StarSystemController {
 public:
    StarSystemController(core::Universe &, engine::Application &, StarSystemCamera &, SysStarSystemRenderer &);
    void Update(float delta_time);

    // Gets the intersection in 3d point between the mouse and any planet
    glm::vec3 GetMouseOnObjectPosition() { return mouse_on_object; }

    /// <summary>
    /// Gets the quaternion to calculate the planet's rotation from the axial rotation
    /// and the rotation period
    /// </summary>
    /// <param name="axial">Axial rotation in radians</param>
    /// <param name="rotation">Rotation period in seconds</param>
    glm::quat GetBodyRotation(double axial, double rotation, double day_offset);

    bool ShouldDrawCityPrototype();

    glm::vec3 CalculateCenteredObject(const entt::entity &);
    glm::vec3 CalculateCenteredObject(const glm::vec3 &);

    glm::vec3 CalculateObjectPos(const entt::entity &ent);

    void PreRender();

    entt::entity m_viewing_entity = entt::null;

 private:
    void SeeEntity();
    void CenterCameraOnCity();
    void MoveCamera(double delta_time);

    void FocusOnEntity(entt::entity ent);
    void SeePlanet(entt::entity ent);

    void CalculateScroll();
    float GetScrollValue();

    void FocusPlanetView();
    void FocusCityView();

    void CalculateCityPositions();
    void CityDetection();

    void CenterCameraOnPoint();
    void SetCameraToPlanetReferenceFrame();
    void SetCameraToSolarSystemReferenceFrame();
    void FoundCity();
    bool IsFoundingCity();

    glm::vec3 CalculateMouseRay(const glm::vec3 &ray_nds);
    entt::entity GetMouseOnObject(int mouse_x, int mouse_y);
    void CalculateViewChange(double deltaX, double deltaY);

    core::components::types::SurfaceCoordinate GetMouseSurfaceIntersection();

    glm::vec3 GetMouseIntersectionOnObject(int mouse_x, int mouse_y);
    glm::vec3 GetMouseInScreenSpace(int mouse_x, int mouse_y);
    std::optional<glm::vec3> CheckIntersection(const glm::vec3 &object_pos, const glm::vec3 &ray_wor, float radius);

    void SelectProvince();

    core::Universe &universe;
    engine::Application &app;

    StarSystemCamera &camera;
    SysStarSystemRenderer &renderer;

    entt::entity terrain_displaying = entt::null;
    entt::entity selected_city = entt::null;

    double previous_mouseX;
    double previous_mouseY;

    entt::entity on_planet = entt::null;

    /// <summary>
    /// Debugging mouse position
    /// </summary>
    int tex_x;
    int tex_y;

    /// <summary>
    /// Debugging colors
    /// </summary>
    int tex_r;
    int tex_g;
    int tex_b;

    entt::entity hovering_province;
    entt::entity selected_province;

    glm::vec3 mouse_on_object;

    bool is_rendering_founding_city;
    bool is_founding_city;

    const float CAMERA_MOVEMENT_SPEED = 30.f / 40.f;
    const float PAN_SPEED = 4.0f;
    const float SCROLL_SENSITIVITY = 3.f / 33.f;

    bool focus_on_city = false;
    bool planet_frame_scroll = false;
    core::components::types::SurfaceCoordinate target_surface_coordinate;

    core::components::types::SurfaceCoordinate GetCameraOverCoordinate();

    friend StarSystemViewUI;

    const glm::vec4 selected_province_color = glm::vec4(1.f, 0.f, 0.f, 0.35f);
};
}  // namespace cqsp::client::systems
