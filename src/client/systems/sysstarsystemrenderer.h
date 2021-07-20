/*
 * Copyright 2021 Conquer Space
*/
#pragma once

#include <noiseutils.h>

#include <entt/entt.hpp>

#include <glm/glm.hpp>

#include "client/systems/sysplanetterraingenerator.h"
#include "common/universe.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/texturerenderer.h"
#include "engine/renderer/renderer.h"
#include "engine/application.h"

namespace conquerspace {
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
    SysStarSystemRenderer(conquerspace::common::components::Universe &,
                          conquerspace::engine::Application &);
    void Initialize();
    void Render();
    void SeeStarSystem(entt::entity);
    void SeeEntity();

    float view_x;
    float view_y;

    float scroll = 10;

    glm::vec3 view_center;

    double GetDivider() { return divider; }

    entt::entity GetMouseOnObject(int mouse_x, int mouse_y);

 private:
    entt::entity m_star_system = entt::null;
    entt::entity m_viewing_entity = entt::null;
    conquerspace::common::components::Universe &m_universe;
    conquerspace::engine::Application &m_app;

    conquerspace::engine::Renderable planet;
    conquerspace::engine::Renderable sky;
    conquerspace::engine::Renderable planet_circle;
    conquerspace::engine::Renderable ship_overlay;
    conquerspace::engine::Renderable sun;

    glm::vec3 cam_pos;
    glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 camera_matrix;
    glm::mat4 projection;
    glm::vec4 viewport;

    float circle_size = 0.01f;

    void DrawEntityName(glm::vec3 &object_pos, entt::entity ent_id);
    void DrawPlanetIcon(glm::vec3 &object_pos);
    void DrawShipIcon(glm::vec3 &object_pos);
    void DrawPlanet(glm::vec3 &object_pos);
    void DrawStar(glm::vec3 &object_pos);
    void DrawTerrainlessPlanet(glm::vec3 &object_pos);

    glm::vec3 CalculateObjectPos(entt::entity &);
    glm::vec3 CalculateCenteredObject(entt::entity &);
    void CalculateCamera();

    void SetPlanetTexture(TerrainImageGenerator &);
    unsigned int GeneratePlanetTexture(noise::utils::Image& image);
    glm::vec3 CalculateMouseRay(glm::vec3 &ray_nds);
    float GetWindowRatio();

    conquerspace::asset::Texture* GenerateTexture(unsigned int, noise::utils::Image&);
    // How much to scale the the star system.
    const double divider = 0.01;

    glm::vec3 sun_position;
    glm::vec3 sun_color;

    std::thread terrain_generator_thread, less_detailed_terrain_generator_thread;

    bool terrain_complete = false, second_terrain_complete = false;

    TerrainImageGenerator final_image_generator, intermediate_image_generator;

    engine::FramebufferRenderer buffer_renderer;
    engine::FramebufferRenderer planet_renderer;
    engine::FramebufferRenderer skybox_renderer;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
