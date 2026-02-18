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

#include <map>
#include <string>
#include <vector>

#include "client/scenes/universe/views/starsystemcamera.h"
#include "client/scenes/universe/views/starsystemcontroller.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "client/scenes/universe/views/starsystemviewui.h"
#include "client/scenes/universe/views/sysorbitgeometry.h"
#include "core/components/coordinates.h"
#include "core/universe.h"
#include "engine/application.h"
#include "engine/graphics/model.h"
#include "engine/graphics/renderable.h"
#include "engine/renderer/framebuffer.h"
#include "engine/renderer/renderer.h"
#include "entt/entt.hpp"
#include "glm/glm.hpp"

namespace cqsp::client::systems {
/*
 * Main renderer for the solar system
 */
class SysStarSystemRenderer {
 public:
    SysStarSystemRenderer(core::Universe &, engine::Application &);
    void Initialize();
    void OnTick();
    void Render(float delta_time);
    void SeeStarSystem();
    void Update(float delta_time);
    void DoUI(float delta_time);

    double previous_mouseX;
    double previous_mouseY;

    double min_zoom = 1;
    // Light year sized
    double max_zoom = 9.4605284e15;

    double GetDivider() { return divider; }

    void DrawAllOrbits();

    ~SysStarSystemRenderer();

 private:
    core::Universe &universe;
    engine::Application &app;

    StarSystemCamera camera;
    StarSystemController controller;
    StarSystemViewUI user_interface;
    SysOrbitGeometry orbit_geometry;

    engine::Renderable textured_planet;
    engine::Renderable sky;
    engine::Renderable planet_circle;
    engine::Renderable ship_overlay;
    engine::Renderable city;
    engine::Renderable sun;

    asset::ShaderProgram_t model_shader;
    asset::ShaderProgram_t orbit_shader;
    asset::ShaderProgram_t near_shader;
    asset::ShaderProgram_t vis_shader;
    asset::ShaderProgram_t circle_shader;
    asset::ShaderProgram_t textured_planet_shader;
    asset::ShaderProgram_t sun_shader;
    asset::ShaderProgram_t skybox_shader;
    asset::ShaderProgram_t buffer_shader;
#if FALSE
    // Disabled for now
    asset::ShaderProgram_t no_light_shader;
#endif

    void DrawStars();
    void DrawBodies();
    void DrawShips();
    void DrawSkybox();
    void DrawModels();

    void DrawEntityName(glm::vec3 &object_pos, entt::entity ent_id);
    void DrawPlanetBillboard(const entt::entity &ent_id, const glm::vec3 &object_pos);
    void DrawShipIcon(const glm::vec3 &object_pos);
    void DrawCityIcon(const glm::vec3 &object_pos, float alpha_value);

    void DrawAllCities(auto &bodies);

    void DrawAllPlanets(auto &bodies);
    void DrawAllPlanetBillboards(auto &bodies);

    void DrawTexturedPlanet(const glm::vec3 &object_pos, const entt::entity entity);
    void GetPlanetTexture(const entt::entity entity, bool &have_normal, bool &have_roughness, bool &have_province);
    void DrawTerrainlessPlanet(const entt::entity &entity, glm::vec3 &object_pos);

    void DrawStar(const entt::entity &entity, glm::vec3 &object_pos);
    void RenderCities(glm::vec3 &object_pos, const entt::entity &body_entity);
    bool CityIsVisible(glm::vec3 city_pos, glm::vec3 planet_pos, glm::vec3 cam_pos);

    asset::ShaderProgram_t ConstructShader(const std::string &key);

    void LoadPlanetTextures();
    void InitializeFramebuffers();
    void LoadProvinceMap();
    void InitializeMeshes();

    void DrawOrbit(const entt::entity &entity);

    glm::vec3 TranslateToNormalized(const glm::vec3 &);

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

    void CheckResourceDistRender();

    float GetWindowRatio();

    float Lerp(float a, float b, float t);

    // How much to scale the the star system.
    const double divider = 0.01;
    float window_ratio;

    const float circle_size = 0.01f;

    glm::vec3 sun_position;
    glm::vec3 sun_color;

    engine::LayerRenderer renderer;

    int ship_icon_layer;
    int planet_icon_layer;
    int physical_layer;
    int skybox_layer;

    float view_scale = 10.f;

    const int sphere_resolution = 64;

    bool have_province = false;

    asset::Texture *dummy_index_texture = nullptr;
    asset::TBOTexture *dummy_color_map = nullptr;

    void SetupDummyTextures();

    void UpdatePlanetProvinceColors(entt::entity body, entt::entity province, glm::vec4 color);
    void MassUpdatePlanetProvinceColors(entt::entity entity);
    void ResetPlanetProvinceColors(entt::entity entity);
    void GeneratePlanetProvinceMap(entt::entity entity, int province_width, int province_height,
                                   uint16_t province_count);
    friend StarSystemViewUI;
    friend StarSystemController;
};
}  // namespace cqsp::client::systems
