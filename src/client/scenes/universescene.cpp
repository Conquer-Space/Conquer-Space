/*
* Copyright 2021 Conquer Space
*/
#include "client/scenes/universescene.h"

#include <fmt/format.h>

#include <cmath>
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "engine/renderer/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/primitives/cube.h"
#include "engine/renderer/primitives/circle.h"
#include "engine/gui.h"

#include "common/components/bodies.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/orbit.h"

conquerspace::scene::UniverseScene::UniverseScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseScene::Init() {
    sphere.mesh = new conquerspace::engine::Mesh();
    conquerspace::primitive::ConstructSphereMesh(64, 64, *sphere.mesh);

    asset::ShaderProgram* program =
        GetApplication().GetAssetManager().CreateShaderProgram("objectvert",
                                                               "defaultfrag");

    asset::Texture* texture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earth");

    asset::Texture* earthNormalTexture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earthnormal");

    asset::Texture* earthRoughnessTexture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earthroughness");

    sphere.shaderProgram = program;
    sphere.SetTexture("texture0", 0, texture);
    sphere.SetTexture("normalMap", 1, earthNormalTexture);
    sphere.SetTexture("roughnessMap", 2, earthRoughnessTexture);

    asset::ShaderProgram* skyboxProgram =
                            GetApplication().GetAssetManager().CreateShaderProgram("skycubevert",
                                                               "skycubefrag");

    asset::Texture* skyTexture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("skycubemap");

    sky.mesh = new conquerspace::engine::Mesh();
    primitive::MakeCube(*sky.mesh);
    sky.shaderProgram = skyboxProgram;
    sky.SetTexture("texture0", 0, skyTexture);

    asset::ShaderProgram* sunshader =
                            GetApplication().GetAssetManager().CreateShaderProgram("objectvert",
                                                                                    "spherefrag");
    sun.shaderProgram = sunshader;
    sun.mesh = sphere.mesh;

    asset::ShaderProgram* circleShader =
                            GetApplication().GetAssetManager().
                            CreateShaderProgram("shader.pane.vert",
                                                "coloredcirclefrag");
    circleShader->UseProgram();
    circleShader->setVec4("color", 1, 0, 1, 1);
    planetDisp.mesh = new conquerspace::engine::Mesh();
    conquerspace::primitive::CreateFilledCircle(*planetDisp.mesh);
    planetDisp.shaderProgram = circleShader;

    x = 0;
    y = -distance;

    previous_mouseX = GetApplication().GetMouseX();
    previous_mouseY = GetApplication().GetMouseY();

    // Get star system, display stuff
    auto civilizationView =
        GetApplication().GetUniverse().registry.
                view<conquerspace::components::Civilization, conquerspace::components::Player>();
    conquerspace::components::Civilization c;
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
        c = civ;
    }

    // Initialize star system

    conquerspace::components::bodies::Body body = GetApplication().GetUniverse().
                        registry.get<conquerspace::components::bodies::Body>(c.starting_planet);
    conquerspace::components::bodies::StarSystem system = GetApplication().GetUniverse().
                    registry.get<conquerspace::components::bodies::StarSystem>(body.star_system);
    spdlog::info("{}", body.star_system);
    // Add tag to star system
    for (auto a : system.bodies) {
        GetApplication().GetUniverse().registry.emplace<ToRender>(a);
    }
}

void conquerspace::scene::UniverseScene::Update(float deltaTime) {
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (GetApplication().ButtonIsHeld(GLFW_KEY_A)) {
            x += (deltaTime * 10);
        }
        if (GetApplication().ButtonIsHeld(GLFW_KEY_D)) {
            x -= (deltaTime * 10);
        }
        if (GetApplication().ButtonIsHeld(GLFW_KEY_W)) {
            y += (deltaTime * 10);
        }
        if (GetApplication().ButtonIsHeld(GLFW_KEY_S)) {
            y -= (deltaTime * 10);
        }
    }

    double deltaX = previous_mouseX - GetApplication().GetMouseX();
    double deltaY = previous_mouseY - GetApplication().GetMouseY();
    if (!ImGui::GetIO().WantCaptureMouse) {
        if (scroll + GetApplication().GetScrollAmount()*3 > 1.5
                    /*&& scroll + GetApplication().GetScrollAmount() < 95*/) {
            scroll += GetApplication().GetScrollAmount() * 3;
        }

        // Now do things with it
        if (deltaY > 3.1415/2) {
            deltaY = 3.1415/2;
        }
        if (deltaY < -3.1415/2) {
            deltaY = -3.1415/2;
        }

        x += deltaX;
        y += deltaY;
        if (GetApplication().MouseButtonIsHeld(GLFW_MOUSE_BUTTON_LEFT)) {
            viewAngleX += deltaX/GetApplication().GetWindowWidth()*3.1415*4;
            viewAngleY -= deltaY/GetApplication().GetWindowHeight()*3.1415*4;
            if (glm::degrees(viewAngleY) > 89.f) {
                viewAngleY = glm::radians(89.f);
            }
            if (glm::degrees(viewAngleY) < -89.f) {
                viewAngleY = glm::radians(-89.f);
            }
        }

        previous_mouseX = GetApplication().GetMouseX();
        previous_mouseY = GetApplication().GetMouseY();
    }
}

void conquerspace::scene::UniverseScene::Ui(float deltaTime) {
    ImGui::Begin("Window");

    auto civ = GetApplication().GetUniverse().
                                    registry.get<conquerspace::components::Civilization>(player);
    ImGui::Text("%d", civ.starting_planet);
    conquerspace::components::bodies::Body body = GetApplication().GetUniverse().
                        registry.get<conquerspace::components::bodies::Body>(civ.starting_planet);
    conquerspace::components::bodies::StarSystem system = GetApplication().GetUniverse().
                    registry.get<conquerspace::components::bodies::StarSystem>(body.star_system);
    ImGui::Text("%d", system.bodies.size());
    ImGui::Text(fmt::format("{} {}", viewAngleX, viewAngleY).c_str());
    // Iterate through the objects, and print out the orbits
    for (auto bod : system.bodies) {
        conquerspace::components::bodies::Orbit orbit = GetApplication().GetUniverse().
                                        registry.get<conquerspace::components::bodies::Orbit>(bod);
        conquerspace::components::bodies::Vec2 vec = \
                                                conquerspace::components::bodies::toVec2(orbit);
                ImGui::Text(fmt::format("Body {}", bod).c_str());

        ImGui::Text(fmt::format("{}, {}", vec.x, vec.y).c_str());
        ImGui::Text(fmt::format("{}, {}, {}, {}", orbit.argument,
                                                    orbit.eccentricity,
                                                    orbit.theta,
                                                    orbit.semiMajorAxis).c_str());
    }
    ImGui::End();
}

void conquerspace::scene::UniverseScene::Render(float deltaTime) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto& civ = GetApplication().GetUniverse().
                                    registry.get<conquerspace::components::Civilization>(player);
    conquerspace::components::bodies::Orbit& focusPosition = GetApplication().GetUniverse().
                        registry.get<conquerspace::components::bodies::Orbit>(civ.starting_planet);
    conquerspace::components::bodies::Vec2& focusPoint =
                                        conquerspace::components::bodies::toVec2(focusPosition);

    // Set view
    glm::mat4 projection =
              glm::infinitePerspective(glm::radians(45.f),
                                static_cast<float>(GetApplication().GetWindowWidth()) /
                                static_cast<float>(GetApplication().GetWindowHeight()), 0.1f);

    float divider = 1000000.f;
    glm::vec3 camPos = glm::vec3(
                cos(viewAngleY) * sin(viewAngleX) * scroll,
                sin(viewAngleY) * scroll,
                cos(viewAngleY) * cos(viewAngleX) * scroll);

    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 focusVec = glm::vec3(focusPoint.x / divider, 0, focusPoint.y / divider);
    camPos = camPos + focusVec;

    glm::mat4 cameraMatrix = glm::lookAt(camPos, focusVec, camUp);

    sun.shaderProgram->UseProgram();
    sun.shaderProgram->setMat4("projection", projection);
    sun.shaderProgram->setMat4("view", cameraMatrix);

    // Iterate through the objects, and print out the orbits
    // Get star
    glm::vec4 viewport = glm::vec4(0.f, 0.f,
                            GetApplication().GetWindowWidth(), GetApplication().GetWindowHeight());

    auto system = GetApplication().GetUniverse().registry.
        view<ToRender, conquerspace::components::bodies::Body>(
                                    entt::exclude<conquerspace::components::bodies::LightEmitter>);

    planetDisp.shaderProgram->UseProgram();
    planetDisp.shaderProgram->setVec4("color", 1, 0, 1, 1);
    for (auto [bod, body] : system.each()) {
        conquerspace::components::bodies::Orbit& orbit = GetApplication().GetUniverse().
                                        registry.get<conquerspace::components::bodies::Orbit>(bod);
        conquerspace::components::bodies::Vec2& vec =
                                                conquerspace::components::bodies::toVec2(orbit);

        glm::vec3 objectPos = glm::vec3(vec.x / divider, 0, vec.y / divider);
        if (glm::distance(camPos, objectPos) < 200) {
            glm::mat4 position = glm::mat4(1.f);
            position = glm::translate(position, objectPos);

            glm::mat4 transform = glm::mat4(1.f);
            transform = glm::scale(transform, glm::vec3(1, -1, 1));
            sun.shaderProgram->UseProgram();

            position = position * transform;
            sun.shaderProgram->setMat4("model", position);
            sun.shaderProgram->setMat4("view", cameraMatrix);
            sun.shaderProgram->setMat4("projection", projection);

            sun.shaderProgram->setVec3("lightDir", glm::normalize(glm::vec3(0, 0, 0) - objectPos));
            sun.shaderProgram->setVec3("lightPosition", glm::vec3(0, 0, 0));

            sun.shaderProgram->setVec3("lightColor", 1, 1, 1);
            sun.shaderProgram->setVec3("albedo", 1, 0, 1);
            sun.shaderProgram->setVec3("viewPos", camPos);
            sun.shaderProgram->setFloat("metallic", 0.5);
            sun.shaderProgram->setFloat("roughness", 0.1);
            sun.shaderProgram->setFloat("ao", 1.0);

            engine::Draw(sun);
        } else {
            glm::vec3 pos = glm::project(objectPos, cameraMatrix,
                                         projection, viewport);
            glm::mat4 planetDispMat = glm::mat4(1.0f);

            planetDispMat = glm::translate(
                planetDispMat,
                glm::vec3(
                    (pos.x / GetApplication().GetWindowWidth() - 0.5) * 2,
                    (pos.y / GetApplication().GetWindowHeight() - 0.5) * 2, 0));
            planetDispMat =
                glm::scale(planetDispMat, glm::vec3(0.01, 0.01, 0.01));
            planetDispMat = glm::scale(
                planetDispMat,
                glm::vec3(
                    1,
                    static_cast<float>(GetApplication().GetWindowWidth()) /
                        static_cast<float>(GetApplication().GetWindowHeight()),
                    1));
            glm::mat4 twodimproj = glm::scale(
                glm::mat4(1.0f),
                glm::vec3(
                    1,
                    static_cast<float>(GetApplication().GetWindowWidth()) /
                        static_cast<float>(GetApplication().GetWindowHeight()),
                    1));

            twodimproj = glm::mat4(1.0f);
            planetDisp.shaderProgram->UseProgram();
            planetDisp.shaderProgram->setMat4("model", planetDispMat);
            planetDisp.shaderProgram->setMat4("projection", twodimproj);

            if (civ.starting_planet == bod) {
                planetDisp.shaderProgram->setVec4("color", 0, 1, 0, 1);
            } else {
                planetDisp.shaderProgram->setVec4("color", 1, 0, 1, 1);
            }

            engine::Draw(planetDisp);

            std::string isplayet = "planet";
            if (GetApplication()
                    .GetUniverse()
                    .registry.all_of<conquerspace::components::bodies::Star>(
                        bod)) {
                isplayet = "star";
            }

            GetApplication().DrawText(fmt::format("{} {}",
                                        isplayet, bod), pos.x+20, pos.y);
        }
    }

    // Draw stars
    auto stars = GetApplication().GetUniverse().registry.
        view<ToRender, conquerspace::components::bodies::Body,
        conquerspace::components::bodies::LightEmitter>();

    for (auto [bod, body] : stars.each()) {
        conquerspace::components::bodies::Orbit& orbit = GetApplication().GetUniverse().
                                        registry.get<conquerspace::components::bodies::Orbit>(bod);
        conquerspace::components::bodies::Vec2& vec =
                                                conquerspace::components::bodies::toVec2(orbit);
                glm::vec3 objectPos = glm::vec3(vec.x / divider, 0, vec.y / divider);

    glm::vec3 pos = glm::project(objectPos, cameraMatrix,
                                        projection, viewport);

        glm::mat4 planetDispMat = glm::mat4(1.0f);

        planetDispMat = glm::translate(
            planetDispMat,
            glm::vec3(
                (pos.x / GetApplication().GetWindowWidth() - 0.5) * 2,
                (pos.y / GetApplication().GetWindowHeight() - 0.5) * 2, 0));
        planetDispMat =
            glm::scale(planetDispMat, glm::vec3(0.01, 0.01, 0.01));
        planetDispMat = glm::scale(
            planetDispMat,
            glm::vec3(
                1,
                static_cast<float>(GetApplication().GetWindowWidth()) /
                    static_cast<float>(GetApplication().GetWindowHeight()),
                1));
        glm::mat4 twodimproj = glm::scale(
            glm::mat4(1.0f),
            glm::vec3(
                1,
                static_cast<float>(GetApplication().GetWindowWidth()) /
                    static_cast<float>(GetApplication().GetWindowHeight()),
                1));

        twodimproj = glm::mat4(1.0f);
        planetDisp.shaderProgram->UseProgram();
        planetDisp.shaderProgram->setMat4("model", planetDispMat);
        planetDisp.shaderProgram->setMat4("projection", twodimproj);
        planetDisp.shaderProgram->setVec4("color", 1, 1, 0, 1);


        engine::Draw(planetDisp);

        std::string isplayet = "planet";
        if (GetApplication()
                .GetUniverse()
                .registry.all_of<conquerspace::components::bodies::Star>(
                    bod)) {
            isplayet = "star";
        }
        GetApplication().DrawText(fmt::format("{}",
                                    isplayet), pos.x+20, pos.y);
    }

    sky.shaderProgram->UseProgram();

    // Remove position matrix
    sky.shaderProgram->setMat4("view",
                        glm::mat4(glm::mat3(cameraMatrix)));
    sky.shaderProgram->setMat4("projection", projection);

    glDepthFunc(GL_LEQUAL);
    // skybox cube
    engine::Draw(sky);
    glDepthFunc(GL_LESS);
}
