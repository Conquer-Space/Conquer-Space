/*
 * Copyright 2021 Conquer Space
 */
#include "client/systems/sysstarsystemrenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "engine/renderer/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/primitives/cube.h"
#include "engine/renderer/primitives/circle.h"

#include "common/components/bodies.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/orbit.h"

conquerspace::client::SysStarSystemRenderer::SysStarSystemRenderer
                                                    (conquerspace::components::Universe &_u,
                                                    conquerspace::engine::Application &_a) :
                                                    m_universe(_u), m_app(_a),
                                                    scroll(5), view_x(0),
                                                    view_y(0), view_center(glm::vec3(1, 1, 1)) {
}

void conquerspace::client::SysStarSystemRenderer::Initialize() {
    // Initialize meshes, etc
    conquerspace::engine::Mesh* sphere_mesh = new conquerspace::engine::Mesh();
    conquerspace::primitive::ConstructSphereMesh(64, 64, *sphere_mesh);

    // Initialize shaders
    asset::ShaderProgram* planet_shader =
                            m_app.GetAssetManager().CreateShaderProgram("objectvert",
                                                                        "spherefrag");

    asset::ShaderProgram* star_shader =
                            m_app.GetAssetManager().CreateShaderProgram("objectvert",
                                                                        "sunshader");

    asset::ShaderProgram* circle_shader =
                        m_app.GetAssetManager().
                        CreateShaderProgram("shader.pane.vert",
                                            "coloredcirclefrag");

    asset::ShaderProgram* skybox_shader =
                            m_app.GetAssetManager().CreateShaderProgram("skycubevert",
                                                               "skycubefrag");

    // Initialize sky box
    asset::Texture* sky_texture = m_app.GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("skycubemap");

    sky.mesh = new conquerspace::engine::Mesh();
    primitive::MakeCube(*sky.mesh);
    sky.shaderProgram = skybox_shader;
    sky.SetTexture("texture0", 0, sky_texture);

    planet_circle.mesh = new conquerspace::engine::Mesh();
    primitive::CreateFilledCircle(*planet_circle.mesh);
    planet_circle.shaderProgram = circle_shader;

    // Planet spheres
    planet.mesh = sphere_mesh;
    planet.shaderProgram = planet_shader;

    sun.mesh = sphere_mesh;
    sun.shaderProgram = star_shader;
}

void conquerspace::client::SysStarSystemRenderer::Render() {
    namespace cqspb = conquerspace::components::bodies;

    glm::vec3 cam_pos = glm::vec3(
                cos(view_y) * sin(view_x) * scroll,
                sin(view_y) * scroll,
                cos(view_y) * cos(view_x) * scroll);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 focusVec = glm::vec3(view_center.x, view_center.y, view_center.z);
    cam_pos = cam_pos + focusVec;
    glm::mat4 cameraMatrix = glm::lookAt(cam_pos, focusVec, camUp);

    // Set projection
    glm::mat4 projection =
              glm::infinitePerspective(glm::radians(45.f),
                                static_cast<float>(m_app.GetWindowWidth()) /
                                static_cast<float>(m_app.GetWindowHeight()), 0.1f);
    glm::vec4 viewport = glm::vec4(0.f, 0.f,
                            m_app.GetWindowWidth(), m_app.GetWindowHeight());

    // Draw stars
    auto stars = m_app.GetUniverse().registry.
        view<ToRender, conquerspace::components::bodies::Body,
                                                conquerspace::components::bodies::LightEmitter>();
    for (auto [ent_id, body] : stars.each()) {
        // Draw the planet circle
        conquerspace::components::bodies::Orbit& orbit = m_app.GetUniverse().
                                    registry.get<conquerspace::components::bodies::Orbit>(ent_id);
        conquerspace::components::bodies::Vec2& vec =
                                                conquerspace::components::bodies::toVec2(orbit);
        glm::vec3 object_pos = glm::vec3(vec.x / divider, 0, vec.y / divider);

        DrawStar(object_pos, cameraMatrix, projection, cam_pos);
    }

    // Draw other bodies
    auto bodies = m_app.GetUniverse().registry.
        view<ToRender, conquerspace::components::bodies::Body>(
                entt::exclude<conquerspace::components::bodies::LightEmitter>);
    for (auto [ent_id, body] : bodies.each()) {
        // Draw the planet circle
        conquerspace::components::bodies::Orbit& orbit = m_app.GetUniverse().
                                    registry.get<conquerspace::components::bodies::Orbit>(ent_id);
        conquerspace::components::bodies::Vec2& vec =
                                                conquerspace::components::bodies::toVec2(orbit);
        glm::vec3 object_pos = glm::vec3(vec.x / divider, 0, vec.y / divider);

        planet_circle.shaderProgram->UseProgram();
        planet_circle.shaderProgram->setVec4("color", 1, 0, 1, 1);
        if(glm::distance(object_pos, cam_pos) > 200) {
            // Check if it's obscured by a planet, but eh, we can deal with it later
            DrawPlanetIcon(object_pos, cameraMatrix, projection, viewport);
            continue;
        }
        DrawPlanet(object_pos, cameraMatrix, projection, cam_pos);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw sky
    sky.shaderProgram->UseProgram();
    sky.shaderProgram->setMat4("view",
                        glm::mat4(glm::mat3(cameraMatrix)));
    sky.shaderProgram->setMat4("projection", projection);

    glDepthFunc(GL_LEQUAL);
    // skybox cube
    engine::Draw(sky);
    glDepthFunc(GL_LESS);
}

void conquerspace::client::SysStarSystemRenderer::SeeStarSystem(
    entt::entity system) {
    namespace cqspb = conquerspace::components::bodies;
    if (m_star_system != entt::null &&
        m_universe.registry.all_of<cqspb::StarSystem>(m_star_system)) {
        // Remove tags
        auto star_system_component =
            m_universe.registry.get<cqspb::StarSystem>(m_star_system);
        for (auto body : star_system_component.bodies) {
            // Add a tag
            m_universe.registry.remove_if_exists<ToRender>(body);
        }
    }

    m_star_system = system;
    auto star_system_component = m_universe.registry.get<cqspb::StarSystem>(m_star_system);
    for (auto body : star_system_component.bodies) {
        // Add a tag
        m_universe.registry.emplace_or_replace<ToRender>(body);
    }
}

void conquerspace::client::SysStarSystemRenderer::DrawPlanetIcon(
    glm::vec3 &object_pos, glm::mat4 &cameraMatrix, glm::mat4 &projection, glm::vec4 &viewport) {
    glm::vec3 pos = glm::project(object_pos, cameraMatrix,
                                        projection, viewport);
    glm::mat4 planetDispMat = glm::mat4(1.0f);

    planetDispMat = glm::translate(
        planetDispMat,
        glm::vec3(
            (pos.x / m_app.GetWindowWidth() - 0.5) * 2,
            (pos.y / m_app.GetWindowHeight() - 0.5) * 2, 0));
    planetDispMat =
        glm::scale(planetDispMat, glm::vec3(0.01, 0.01, 0.01));
    planetDispMat = glm::scale(
        planetDispMat,
        glm::vec3(
            1,
            static_cast<float>(m_app.GetWindowWidth()) /
                static_cast<float>(m_app.GetWindowHeight()),
            1));
    glm::mat4 twodimproj = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(
            1,
            static_cast<float>(m_app.GetWindowWidth()) /
                static_cast<float>(m_app.GetWindowHeight()),
            1));

    twodimproj = glm::mat4(1.0f);
    planet_circle.shaderProgram->UseProgram();
    planet_circle.shaderProgram->setMat4("model", planetDispMat);
    planet_circle.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(planet_circle);
}

void conquerspace::client::SysStarSystemRenderer::DrawPlanet(glm::vec3 &object_pos,
                                                            glm::mat4 &cameraMatrix,
                                                            glm::mat4 &projection,
                                                            glm::vec3 &cam_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    // For some reason, the sphere we make needs to be inverted
    transform = glm::scale(transform, glm::vec3(1, -1, 1));

    position = position * transform;

    planet.shaderProgram->UseProgram();
    planet.shaderProgram->setMat4("model", position);
    planet.shaderProgram->setMat4("view", cameraMatrix);
    planet.shaderProgram->setMat4("projection", projection);

    planet.shaderProgram->setVec3("lightDir", glm::normalize(glm::vec3(0, 0, 0) - object_pos));
    planet.shaderProgram->setVec3("lightPosition", glm::vec3(0, 0, 0));

    planet.shaderProgram->setVec3("lightColor", 1, 1, 1);
    planet.shaderProgram->setVec3("albedo", 1, 0, 1);
    planet.shaderProgram->setVec3("viewPos", cam_pos);
    planet.shaderProgram->setFloat("metallic", 0.5);
    planet.shaderProgram->setFloat("roughness", 0.1);
    planet.shaderProgram->setFloat("ao", 1.0);

    engine::Draw(planet);
}

void conquerspace::client::SysStarSystemRenderer::DrawStar(
    glm::vec3 &object_pos, glm::mat4 &cameraMatrix, glm::mat4 &projection,
    glm::vec3 &cam_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    // For some reason, the sphere we make needs to be inverted
    transform = glm::scale(transform, glm::vec3(1, -1, 1));
    transform = glm::scale(transform, glm::vec3(10, 10, 10));

    position = position * transform;

    sun.shaderProgram->UseProgram();
    sun.shaderProgram->setMat4("model", position);
    sun.shaderProgram->setMat4("view", cameraMatrix);
    sun.shaderProgram->setMat4("projection", projection);
    engine::Draw(sun);
}
