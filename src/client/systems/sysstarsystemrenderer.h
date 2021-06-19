/*
 * Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>

#include "client/systems/sysplanetterraingenerator.h"
#include "common/universe.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"
#include "engine/application.h"

namespace conquerspace {
namespace client {
namespace systems {
/*
 * Tag class for bodies to render.
 */
struct ToRender {};

/*
 * Main renderer for the universe
 */
class SysStarSystemRenderer {
 public:
    SysStarSystemRenderer(conquerspace::components::Universe &,
                          conquerspace::engine::Application &);
    void Initialize();
    void Render();
    void SeeStarSystem(entt::entity);
    void SeeEntity(entt::entity);

    float view_x;
    float view_y;

    float scroll = 10;

    glm::vec3 view_center;

    double GetDivider() { return divider; }

    entt::entity GetMouseOnObject(int mouse_x, int mouse_y);

 private:
    entt::entity m_star_system = entt::null;
    conquerspace::components::Universe &m_universe;
    conquerspace::engine::Application &m_app;

    conquerspace::engine::Renderable planet;
    conquerspace::engine::Renderable sky;
    conquerspace::engine::Renderable planet_circle;
    conquerspace::engine::Renderable sun;

    std::thread thread;

    bool terrain_complete = false;

    glm::vec3 cam_pos;
    glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 focus_vec;
    glm::mat4 camera_matrix;
    glm::mat4 projection;
    glm::vec4 viewport;

    float circle_size = 0.01f;

    void DrawPlanetIcon(glm::vec3 &object_pos, glm::mat4 &cameraMatrix,
                        glm::mat4 &projection, glm::vec4 &viewport);
    void DrawPlanet(glm::vec3 &object_pos, glm::mat4 &cameraMatrix,
                    glm::mat4 &projection, glm::vec3 &cam_pos);
    void DrawStar(glm::vec3 &object_pos, glm::mat4 &cameraMatrix,
                  glm::mat4 &projection, glm::vec3 &cam_pos);

    glm::vec3 CalculateObjectPos(entt::entity &);
    void CalculateCamera();

    // How much to scale the the star system.
    const double divider = 0.01;

    glm::vec3 sun_position;

    TerrainImageGenerator image_generator;
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
