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
#include "common/components/population.h"
#include "common/components/area.h"
#include "common/components/surface.h"
#include "common/components/name.h"
#include "common/components/resource.h"

#include "client/systems/sysplanetviewer.h"
#include "client/systems/systurnsavewindow.h"

conquerspace::scene::UniverseScene::UniverseScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseScene::Init() {
    namespace cqspb = conquerspace::components::bodies;
    simulation = new conquerspace::systems::simulation::Simulation(GetApplication().GetUniverse());

    system_renderer = new conquerspace::client::systems::SysStarSystemRenderer(
        GetApplication().GetUniverse(), GetApplication());
    system_renderer->Initialize();

    auto civilizationView =
        GetApplication().GetUniverse().
                view<conquerspace::components::Civilization, conquerspace::components::Player>();
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
        player_civ = &civ;
    }
    cqspb::Body body = GetApplication().GetUniverse().
                        get<cqspb::Body>(player_civ->starting_planet);
    system_renderer->SeeStarSystem(body.star_system);
    star_system = &GetApplication().GetUniverse().
                        get<cqspb::StarSystem>(body.star_system);

    system_renderer->SeeEntity(player_civ->starting_planet);
    selected_planet = player_civ->starting_planet;

    AddUISystem<conquerspace::client::systems::SysPlanetInformation>();
    AddUISystem<conquerspace::client::systems::SysTurnSaveWindow>();
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

    // Check for last tick
    if (GetApplication().GetUniverse().ToTick()) {
        // Game tick
        simulation->tick();
    }
    GetApplication().GetUniverse().clear<conquerspace::client::systems::MouseOverEntity>();
    system_renderer->GetMouseOnObject(GetApplication().GetMouseX(), GetApplication().GetMouseY());
    for (auto& ui : user_interfaces) {
        ui->DoUpdate(deltaTime);
    }
}

void conquerspace::scene::UniverseScene::Ui(float deltaTime) {
    for (auto& ui : user_interfaces) {
        ui->DoUI(deltaTime);
    }

    /*int size = 20;
    auto draw = ImGui::GetForegroundDrawList();
    for (int x = 0; x < GetApplication().GetWindowWidth(); x+=size) {
        for (int y = 0; y < GetApplication().GetWindowHeight(); y+=size) {
            ImVec2 vec(x, y);
            if (system_renderer->GetMouseOnObject(x, y) != entt::null) {
                draw->AddRectFilled(vec, ImVec2(x + 1, y + 1),
                                    IM_COL32(255, 0, 0, 255));
            }
        }
    }*/
}

void conquerspace::scene::UniverseScene::Render(float deltaTime) {
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render();
}

