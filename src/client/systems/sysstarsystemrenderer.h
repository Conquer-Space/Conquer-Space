/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <noiseutils.h>

#include <thread>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "client/systems/sysplanetterraingenerator.h"
#include "common/universe.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/texturerenderer.h"
#include "engine/renderer/renderer.h"
#include "engine/application.h"

namespace cqsp {
namespace client {
namespace systems {
/*
 * Tag class for bodies to render.
 */
struct ToRender {};

struct MouseOverEntity {};

struct RenderingStarSystem {};

struct RenderingPlanet {};

/*
 * Main renderer for the universe
 */
class SysStarSystemRenderer {
 public:
    SysStarSystemRenderer(cqsp::common::Universe &,
                          cqsp::engine::Application &);
    void Initialize();
    void OnTick();
    void Render();
    void SeeStarSystem(entt::entity);
    void SeeEntity();
    void Update();
    void SeePlanet(entt::entity);
    void DoUI();

    float view_x;
    float view_y;

    double previous_mouseX;
    double previous_mouseY;

    float scroll = 10;

    glm::vec3 view_center;

    double GetDivider() { return divider; }

    entt::entity GetMouseOnObject(int mouse_x, int mouse_y);

    ~SysStarSystemRenderer();

 private:
    entt::entity m_star_system = entt::null;
    entt::entity m_viewing_entity = entt::null;
    entt::entity terrain_displaying = entt::null;

    cqsp::common::Universe &m_universe;
    cqsp::engine::Application &m_app;

    cqsp::engine::Renderable planet;
    cqsp::engine::Renderable sky;
    cqsp::engine::Renderable planet_circle;
    cqsp::engine::Renderable ship_overlay;
    cqsp::engine::Renderable city;
    cqsp::engine::Renderable sun;

    asset::ShaderProgram_t pbr_shader;
    asset::ShaderProgram_t no_light_shader;

    cqsp::asset::Texture* planet_texture;
    cqsp::asset::Texture* planet_heightmap;
    cqsp::asset::Texture* planet_resource;

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
    void DrawShipIcon(glm::vec3 &object_pos);
    void DrawCityIcon(glm::vec3 &object_pos);
    void DrawPlanet(entt::entity terrain, glm::vec3 &object_pos);
    void DrawStar(glm::vec3 &object_pos);
    void DrawTerrainlessPlanet(glm::vec3 &object_pos);
    void RenderCities(glm::vec3 &object_pos, const entt::entity &body_entity);
    void CalculateCityPositions();

    glm::vec3 CalculateObjectPos(const entt::entity &);
    glm::vec3 CalculateCenteredObject(const entt::entity &);
    glm::vec3 CalculateCenteredObject(const glm::vec3 &);
    glm::vec3 TranslateToNormalized(const glm::vec3 &);
    void CalculateCamera();

    void SetPlanetTexture(TerrainImageGenerator &);
    unsigned int GeneratePlanetTexture(noise::utils::Image& image);
    glm::vec3 CalculateMouseRay(const glm::vec3 &ray_nds);
    float GetWindowRatio();

    cqsp::asset::Texture* GenerateTexture(unsigned int, noise::utils::Image&);
    // How much to scale the the star system.
    const double divider = 0.01;
    float window_ratio;

    glm::vec3 sun_position;
    glm::vec3 sun_color;

    std::thread terrain_generator_thread, less_detailed_terrain_generator_thread;

    bool terrain_complete = false, second_terrain_complete = false;

    TerrainImageGenerator final_image_generator, intermediate_image_generator;

    engine::LayerRenderer renderer;

    int ship_icon_layer;
    int planet_icon_layer;
    int physical_layer;
    int skybox_layer;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
