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
#include "common/components/surface.h"
#include "common/components/name.h"

conquerspace::scene::UniverseScene::UniverseScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseScene::Init() {
    namespace cqspb = conquerspace::components::bodies;
    simulation = new conquerspace::systems::simulation::Simulation(GetApplication().GetUniverse());

    system_renderer = new conquerspace::client::SysStarSystemRenderer(
        GetApplication().GetUniverse(), GetApplication());
    system_renderer->Initialize();

    auto civilizationView =
        GetApplication().GetUniverse().registry.
                view<conquerspace::components::Civilization, conquerspace::components::Player>();
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
        player_civ = &civ;
    }
    cqspb::Body body = GetApplication().GetUniverse().
                        registry.get<cqspb::Body>(player_civ->starting_planet);
    system_renderer->SeeStarSystem(body.star_system);
    star_system = &GetApplication().GetUniverse().
                        registry.get<cqspb::StarSystem>(body.star_system);

    // Set view center
    cqspb::Orbit& orbit = GetApplication().GetUniverse().registry.
                                get<cqspb::Orbit>(player_civ->starting_planet);
    cqspb::Vec2& vec = cqspb::toVec2(orbit);
    system_renderer->view_center = glm::vec3(vec.x / system_renderer->GetDivider(),
                                                        0, vec.y / system_renderer->GetDivider());
    selected_planet = player_civ->starting_planet;
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

        // If clicked on a planet, go to the planet
        entt::entity ent = entt::null;

        if (GetApplication().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
                    (ent = system_renderer->GetMouseOnObject(GetApplication().GetMouseX(),
                    GetApplication().GetMouseY())) != entt::null) {
            // Go to the place
            conquerspace::components::bodies::Orbit& orbit =
                                GetApplication().GetUniverse().registry.
                                get<conquerspace::components::bodies::Orbit>(ent);
            conquerspace::components::bodies::Vec2& vec =
                                                    conquerspace::components::bodies::toVec2(orbit);
            system_renderer->view_center = glm::vec3(vec.x / system_renderer->GetDivider(),
                                                        0, vec.y / system_renderer->GetDivider());
            selected_planet = ent;
            to_show_planet_window = true;
        }
    }

    simulation->tick();
}

void conquerspace::scene::UniverseScene::Ui(float deltaTime) {
    if (selected_planet != entt::null && to_show_planet_window) {
        ImGui::SetNextWindowSize(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.4f, ImGui::GetIO().DisplaySize.y * 0.7f),
            ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.75f, ImGui::GetIO().DisplaySize.y * 0.6f),
            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Planet info", &to_show_planet_window);
        if (GetApplication().GetUniverse().
                        registry.all_of<conquerspace::components::Name>(selected_planet)) {
            std::string i = GetApplication().GetUniverse().
                                registry.get<conquerspace::components::Name>(selected_planet).name;
            ImGui::Text(fmt::format("Planet {}", i).c_str());
        }

        if (GetApplication().GetUniverse()
                .registry.all_of<conquerspace::components::Habitation>(selected_planet)) {
            auto& habit =
                GetApplication()
                    .GetUniverse()
                    .registry.get<conquerspace::components::Habitation>(
                        selected_planet);
            ImGui::Text(fmt::format("{}", habit.settlements.size()).c_str());

            // List cities
            ImGui::BeginChild("Left pane", ImVec2(150, 0));
            for (int i = 0; i < habit.settlements.size(); i++) {
                const bool is_selected = (selected_city == i);

                entt::entity e = habit.settlements[i];
                std::string name = GetApplication()
                    .GetUniverse()
                    .registry.get<conquerspace::components::Name>(
                        e).name;
                if (ImGui::Selectable(
                        fmt::format("{}", name).c_str(),
                        is_selected)) {
                    // Load city
                    selected_city = i;
                }
                if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(fmt::format("{}", name).c_str());
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::BeginChild("City info", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

            // Show city information
            entt::entity e = habit.settlements[selected_city];
            ImGui::Text(fmt::format("{}", GetApplication()
                    .GetUniverse()
                    .registry.get<conquerspace::components::Name>(
                        e).name).c_str());

            if (GetApplication()
                .GetUniverse()
                .registry.all_of<conquerspace::components::Settlement>(e)) {
                int size = GetApplication()
                    .GetUniverse()
                    .registry.get<conquerspace::components::Settlement>(e).population.size();
                ImGui::Text(fmt::format("{}", size).c_str());

                for (auto b : GetApplication()
                    .GetUniverse()
                    .registry.get<conquerspace::components::Settlement>(e).population) {
                    auto bad_var_name = GetApplication()
                        .GetUniverse()
                        .registry.get<conquerspace::components::PopulationSegment>(b);
                    ImGui::Text(fmt::format("Popsize: {}", bad_var_name.population).c_str());
                }
            } else {
                ImGui::Text(fmt::format("Nothing").c_str());
            }

            ImGui::EndChild();
            ImGui::EndGroup();
        }
        ImGui::End();
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
    system_renderer->Render();
}
