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
    system_renderer = new conquerspace::client::SysStarSystemRenderer(
        GetApplication().GetUniverse(), GetApplication());
    system_renderer->Initialize();

    auto civilizationView =
        GetApplication().GetUniverse().registry.
                view<conquerspace::components::Civilization, conquerspace::components::Player>();
    conquerspace::components::Civilization c;
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
        c = civ;
    }
    conquerspace::components::bodies::Body body = GetApplication().GetUniverse().
                        registry.get<conquerspace::components::bodies::Body>(c.starting_planet);
    system_renderer->SeeStarSystem(body.star_system);

    // Set view center
    conquerspace::components::bodies::Orbit& orbit = GetApplication().GetUniverse().registry.
                                get<conquerspace::components::bodies::Orbit>(c.starting_planet);
    conquerspace::components::bodies::Vec2& vec =
                                            conquerspace::components::bodies::toVec2(orbit);
    system_renderer->view_center = glm::vec3(vec.x / system_renderer->GetDivider(),
                                                        0, vec.y / system_renderer->GetDivider());
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
        if (system_renderer->scroll + GetApplication().GetScrollAmount() * 3 > 1.5) {
            system_renderer->scroll += GetApplication().GetScrollAmount() * 3;
        }

        if (GetApplication().MouseButtonIsHeld(GLFW_MOUSE_BUTTON_LEFT)) {
            system_renderer->view_x += deltaX/GetApplication().GetWindowWidth()*3.1415*4;
            system_renderer->view_y -= deltaY/GetApplication().GetWindowHeight()*3.1415*4;

            if (glm::degrees(system_renderer->view_y) > 89.f) {
                system_renderer->view_y = glm::radians(89.f);
            }
            if (glm::degrees(system_renderer->view_y) < -89.f) {
                system_renderer->view_y = glm::radians(-89.f);
            }
        }

        previous_mouseX = GetApplication().GetMouseX();
        previous_mouseY = GetApplication().GetMouseY();
    }
}

void conquerspace::scene::UniverseScene::Ui(float deltaTime) {
    ImGui::Begin("Window");
    ImGui::End();
}

void conquerspace::scene::UniverseScene::Render(float deltaTime) {
    system_renderer->Render();
}
