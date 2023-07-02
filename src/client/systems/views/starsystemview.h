/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <map>
#include <string>
#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "common/components/coordinates.h"
#include "common/universe.h"
#include "engine/application.h"
#include "engine/graphics/renderable.h"
#include "engine/renderer/framebuffer.h"
#include "engine/renderer/renderer.h"

namespace cqsp {
namespace client {
namespace systems {
// TODO(EhWhoAmI): Would be helpful to move the following structs to a header file.
struct MouseOverEntity {};

// Planet that the camera center is at
struct FocusedPlanet {};
// City to look at
struct FocusedCity {};

struct CityFounding {};

/*
 * Main renderer for the universe
 */
class SysStarSystemRenderer {
 public:
    SysStarSystemRenderer(cqsp::common::Universe &, cqsp::engine::Application &);
    void Initialize();
    void OnTick();
    void Render(float deltaTime);
    void SeeStarSystem();
    void SeeEntity();
    void Update(float deltaTime);
    void SeePlanet(entt::entity);
    void DoUI(float deltaTime);

    glm::vec3 GetMouseIntersectionOnObject(int mouse_x, int mouse_y);

    // The angle the camera is looking from
    float view_x = 0;
    // The angle the camera is looking away from
    float view_y = 0;

    double previous_mouseX;
    double previous_mouseY;

    double scroll = 10;
    double min_zoom = 1;
    // Light year sized
    double max_zoom = 9.4605284e15;

    glm::vec3 view_center;

    double GetDivider() { return divider; }

    entt::entity GetMouseOnObject(int mouse_x, int mouse_y);

    static bool IsFoundingCity(common::Universe &universe);

    void DrawAllOrbits();
    void DrawOrbit(const entt::entity &entity);

    void OrbitEditor();

    ~SysStarSystemRenderer();

 private:
    entt::entity m_viewing_entity = entt::null;
    entt::entity terrain_displaying = entt::null;

    cqsp::common::Universe &m_universe;
    cqsp::engine::Application &m_app;

    cqsp::engine::Renderable planet;
    cqsp::engine::Renderable textured_planet;
    cqsp::engine::Renderable sky;
    cqsp::engine::Renderable planet_circle;
    cqsp::engine::Renderable ship_overlay;
    cqsp::engine::Renderable city;
    cqsp::engine::Renderable sun;

    cqsp::asset::ShaderProgram_t orbit_shader;
    cqsp::asset::ShaderProgram_t near_shader;
#if FALSE
    // Disabled for now
    asset::ShaderProgram_t no_light_shader;
#endif

    glm::vec3 cam_pos;
    glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 camera_matrix;
    glm::mat4 projection;
    glm::vec4 viewport;

    float circle_size = 0.01f;

    void DrawStars();
    void DrawBodies();
    void DrawShips();
    void DrawSkybox();

    void DrawEntityName(glm::vec3 &object_pos, entt::entity ent_id);
    void DrawPlanetIcon(glm::vec3 &object_pos);
    void DrawPlanetBillboards(const entt::entity &ent_id, const glm::vec3 &object_pos);
    void DrawShipIcon(const glm::vec3 &object_pos);
    void DrawCityIcon(const glm::vec3 &object_pos);

    void DrawAllCities(auto &bodies);

    void DrawAllPlanets(auto &bodies);
    void DrawAllPlanetBillboards(auto &bodies);

    void DrawTexturedPlanet(const glm::vec3 &object_pos, const entt::entity entity);
    void GetPlanetTexture(const entt::entity entity, bool &have_normal, bool &have_roughness, bool &have_province);
    void DrawTerrainlessPlanet(const entt::entity &entity, glm::vec3 &object_pos);

    void DrawStar(const entt::entity &entity, glm::vec3 &object_pos);
    void RenderCities(glm::vec3 &object_pos, const entt::entity &body_entity);
    bool CityIsVisible(glm::vec3 city_pos, glm::vec3 planet_pos, glm::vec3 cam_pos, double radius);
    void CalculateCityPositions();
    void CalculateScroll();

    void LoadPlanetTextures();
    void InitializeFramebuffers();
    void LoadProvinceMap();
    void InitializeMeshes();

    void GenerateOrbit(entt::entity body);

    /// <summary>
    /// Gets the quaternion to calculate the planet's rotation from the axial rotation
    /// and the rotation period
    /// </summary>
    /// <param name="axial">Axial rotation in radians</param>
    /// <param name="rotation">Rotation period in seconds</param>
    glm::quat GetBodyRotation(double axial, double rotation, double day_offset);
    void FocusCityView();

    glm::vec3 CalculateObjectPos(const entt::entity &);
    glm::vec3 CalculateCenteredObject(const entt::entity &);
    glm::vec3 CalculateCenteredObject(const glm::vec3 &);
    glm::vec3 TranslateToNormalized(const glm::vec3 &);
    glm::vec3 ConvertPoint(const glm::vec3 &);

    /// <summary>
    /// Calculates the GL position for a log renderbuffer.
    /// </summary>
    glm::vec4 CalculateGLPosition(const glm::vec3 &object_pos);
    /// <summary>
    /// Check if the GL position is within the window
    /// </summary>
    bool GLPositionNotInBounds(const glm::vec4 &gl_Position, const glm::vec3 &pos);
    glm::mat4 GetBillboardMatrix(const glm::vec3 &pos);
    glm::vec3 GetBillboardPosition(const glm::vec3 &object_pos);
    void SetBillboardProjection(cqsp::asset::ShaderProgram_t &shader, glm::mat4 mat);
    void CenterCameraOnCity();

    void CalculateCamera();
    void MoveCamera(double deltaTime);

    void CheckResourceDistRender();

    glm::vec3 CalculateMouseRay(const glm::vec3 &ray_nds);

    void CalculateViewChange(double deltaX, double deltaY);
    void FoundCity();
    void SelectCountry();
    void FocusOnEntity(entt::entity ent);

    float GetWindowRatio();

    void GenerateOrbitLines();

    void RenderInformationWindow(double deltaTime);
    void RenderSelectedObjectInformation();

    // How much to scale the the star system.
    const double divider = 0.01;
    float window_ratio;

    glm::vec3 sun_position;
    glm::vec3 sun_color;

    engine::LayerRenderer renderer;

    int ship_icon_layer;
    int planet_icon_layer;
    int physical_layer;
    int skybox_layer;

    bool is_founding_city = false;
    bool is_rendering_founding_city = false;
    glm::vec3 mouse_on_object;
    // Gets the intersection in 3d point between the mouse and any planet
    glm::vec3 GetMouseOnObject() { return mouse_on_object; }
    entt::entity on_planet;

    float view_scale = 10.f;

    entt::entity selected_city = entt::null;

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

    int province_height = 0;
    int province_width = 0;

    common::components::types::SurfaceCoordinate GetMouseSurfaceIntersection();
    void CityDetection();

    glm::vec3 selected_province_color;
    glm::vec3 selected_country_color;
    entt::entity hovering_province;
    entt::entity selected_province;

    int orbits_generated = 0;

    const int sphere_resolution = 64;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
