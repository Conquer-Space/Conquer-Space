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
                                                                                    "sunshader");
    sun.shaderProgram = sunshader;
    sun.mesh = sphere.mesh;

    x = 0;
    y = -distance;

    previous_mouseX = GetApplication().GetMouseX();
    previous_mouseY = GetApplication().GetMouseY();

    // Get star system, display stuff
    auto civilizationView =
        GetApplication().GetUniverse().registry.
                view<conquerspace::components::Civilization, conquerspace::components::Player>();
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
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
        if (scroll + GetApplication().GetScrollAmount() > 1.5
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
    // Iterate through the objects, and print out the orbits
    for (auto bod : system.bodies) {
        conquerspace::components::bodies::Orbit orbit = GetApplication().GetUniverse().
                                        registry.get<conquerspace::components::bodies::Orbit>(bod);
        conquerspace::components::bodies::Vec2 vec = \
                                                conquerspace::components::bodies::toVec2(orbit);
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

    float divider = 100000.f;
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
    conquerspace::components::bodies::Body body = GetApplication().GetUniverse().
                        registry.get<conquerspace::components::bodies::Body>(civ.starting_planet);
    conquerspace::components::bodies::StarSystem system = GetApplication().GetUniverse().
                    registry.get<conquerspace::components::bodies::StarSystem>(body.star_system);

    glm::vec4 viewport = glm::vec4(0.f, 0.f,
                            GetApplication().GetWindowWidth(), GetApplication().GetWindowHeight());
    for (auto bod : system.bodies) {
        conquerspace::components::bodies::Orbit& orbit = GetApplication().GetUniverse().
                                        registry.get<conquerspace::components::bodies::Orbit>(bod);
        conquerspace::components::bodies::Vec2& vec =
                                                conquerspace::components::bodies::toVec2(orbit);

        glm::vec3 objectPos = glm::vec3(vec.x / divider, 0, vec.y / divider);
        glm::mat4 position = glm::mat4(1.f);
        position = glm::translate(position, objectPos);

        glm::mat4 transform = glm::mat4(1.f);

        glm::vec3 pos = glm::project(objectPos,
            transform * cameraMatrix,
            projection,
            viewport);

        std::string isplayet = "planet";
        if (GetApplication().GetUniverse().
                                registry.all_of<conquerspace::components::bodies::Star>(bod)) {
            isplayet = "star";
        }

        GetApplication().DrawText(fmt::format("{} {} {} {} {}",
                                                isplayet, bod, pos.x, pos.y, pos.z), pos.x, pos.y);

        sun.shaderProgram->UseProgram();

        position = position * transform;
        sun.shaderProgram->setMat4("model", position);

        engine::Draw(sun);
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
