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
#include "client/scenes/universescene.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fmt/format.h>

#include <cmath>
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "engine/renderer/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/primitives/cube.h"
#include "engine/renderer/primitives/polygon.h"
#include "engine/gui.h"

#include "common/components/bodies.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/coordinates.h"
#include "common/components/population.h"
#include "common/components/area.h"
#include "common/components/surface.h"
#include "common/components/name.h"
#include "common/components/resource.h"

#include "client/systems/sysplanetviewer.h"
#include "client/systems/systurnsavewindow.h"
#include "client/systems/sysstarsystemtree.h"
#include "client/systems/syspausemenu.h"
#include "client/systems/sysdebuggui.h"
#include "client/systems/syscommand.h"
#include "client/systems/gui/sysevent.h"

bool game_halted = false;

cqsp::scene::UniverseScene::UniverseScene(
    cqsp::engine::Application& app) : Scene(app) {}

void cqsp::scene::UniverseScene::Init() {
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspco = cqsp::common;
    simulation = std::make_shared<cqspco::systems::simulation::Simulation>
                                        (GetApp().GetUniverse(), GetApp().GetScriptInterface());

    system_renderer = new cqsp::client::systems::SysStarSystemRenderer(
        GetApp().GetUniverse(), GetApp());
    system_renderer->Initialize();

    auto civilizationView =
        GetApp().GetUniverse().
                view<cqspco::components::Civilization,
                cqspco::components::Player>();
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
        player_civ = &civ;
    }
    cqspb::Body body = GetApp().GetUniverse().
                        get<cqspb::Body>(player_civ->starting_planet);
    system_renderer->SeeStarSystem(body.star_system);
    star_system = &GetApp().GetUniverse().
                        get<cqspb::StarSystem>(body.star_system);

    SeeStarSystem(GetApp(), body.star_system);
    SeePlanet(GetApp(), player_civ->starting_planet);

    selected_planet = player_civ->starting_planet;

    AddUISystem<cqsp::client::systems::SysPlanetInformation>();
    AddUISystem<cqsp::client::systems::SysTurnSaveWindow>();
    AddUISystem<cqsp::client::systems::SysStarSystemTree>();
    AddUISystem<cqsp::client::systems::SysPauseMenu>();
    AddUISystem<cqsp::client::systems::SysDebugMenu>();
    AddUISystem<cqsp::client::systems::SysCommand>();
    AddUISystem<cqsp::client::systems::gui::SysEvent>();
    simulation->tick();
}

void cqsp::scene::UniverseScene::Update(float deltaTime) {
    if (!game_halted) {
        system_renderer->Update();
    }

    // Check for last tick
    if (GetApp().GetUniverse().ToTick() && !game_halted) {
        // Game tick
        simulation->tick();
        system_renderer->OnTick();
    }

    GetApp().GetUniverse().clear<cqsp::client::systems::MouseOverEntity>();
    system_renderer->GetMouseOnObject(GetApp().GetMouseX(), GetApp().GetMouseY());

    for (auto& ui : user_interfaces) {
        if (game_halted) {
            ui->window_flags = ImGuiWindowFlags_NoInputs;
        } else {
            ui->window_flags = 0;
        }
        ui->DoUpdate(deltaTime);
    }
}

void cqsp::scene::UniverseScene::Ui(float deltaTime) {
    for (auto& ui : user_interfaces) {
        ui->DoUI(deltaTime);
    }
}

void cqsp::scene::UniverseScene::Render(float deltaTime) {
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render();
}

void cqsp::scene::SeeStarSystem(cqsp::engine::Application& app, entt::entity ent) {
    app.GetUniverse().clear<cqsp::client::systems::RenderingStarSystem>();
    app.GetUniverse().emplace<cqsp::client::systems::RenderingStarSystem>(ent);
}

entt::entity cqsp::scene::GetCurrentViewingPlanet(cqsp::engine::Application& app) {
    return app.GetUniverse().view<cqsp::client::systems::RenderingPlanet>().front();
}

void cqsp::scene::SeePlanet(cqsp::engine::Application& app, entt::entity ent) {
    app.GetUniverse().clear<cqsp::client::systems::RenderingPlanet>();
    app.GetUniverse().emplace<cqsp::client::systems::RenderingPlanet>(ent);
}

void cqsp::scene::SetGameHalted(bool b) { game_halted = b; }

bool cqsp::scene::IsGameHalted() { return game_halted; }
