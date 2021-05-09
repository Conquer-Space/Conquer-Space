/*
 * Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>

#include "common/universe.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"
#include "engine/application.h"

namespace conquerspace {
namespace client {

/*
* Tag class for bodies to render.
*/
struct ToRender { };

/*
 * Main renderer for the universe
*/
class SysStarSystemRenderer{
 public:
    SysStarSystemRenderer(conquerspace::components::Universe &,
                           conquerspace::engine::Application &);
    void Initialize();
    void Render();
    void SeeStarSystem(entt::entity);

    float view_x;
    float view_y;

    float scroll = 10;

    glm::vec3 view_center;

    float GetDivider() { return divider; }

 private:
    entt::entity m_star_system = entt::null;
    conquerspace::components::Universe &m_universe;
    conquerspace::engine::Application &m_app;

    conquerspace::engine::Renderable planet;
    conquerspace::engine::Renderable sky;
    conquerspace::engine::Renderable planet_circle;
    conquerspace::engine::Renderable sun;

    void DrawPlanetIcon(glm::vec3 &object_pos, glm::mat4 &cameraMatrix,
                        glm::mat4 &projection, glm::vec4 &viewport);
    void DrawPlanet(glm::vec3 &object_pos, glm::mat4 &cameraMatrix,
                    glm::mat4 &projection, glm::vec3 &cam_pos);
    void DrawStar(glm::vec3 &object_pos, glm::mat4 &cameraMatrix,
                  glm::mat4 &projection, glm::vec3 &cam_pos);
    // How much to scale the the star system.
    const float divider = 100000.f;
};
}  // namespace client
}  // namespace conquerspace
