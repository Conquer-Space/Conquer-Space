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

#include <fmt/format.h>

#include <cmath>
#include <string>

#include <tracy/Tracy.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "engine/graphics/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/graphics/primitives/cube.h"
#include "engine/graphics/primitives/polygon.h"
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
#include "client/systems/civilizationinfopanel.h"
#include "client/systems/sysfieldviewer.h"
#include "client/systems/systechviewer.h"

// If the game is paused or not, like when escape is pressed
bool game_halted = false;

cqsp::scene::UniverseScene::UniverseScene(cqsp::engine::Application& app) : Scene(app) {}

void cqsp::scene::UniverseScene::Init() {
    ZoneScoped;
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspco = cqsp::common;
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::client::systems;

    using cqspco::systems::simulation::Simulation;
    simulation = std::make_unique<Simulation>(GetApp().GetGame());

    system_renderer = new cqsps::SysStarSystemRenderer(GetUniverse(), GetApp());
    system_renderer->Initialize();

    auto civilizationView = GetUniverse().view<cqspc::Civilization, cqspc::Player>();
    for (auto [entity, civ] : civilizationView.each()) {
        player = entity;
        player_civ = &civ;
    }
    //cqspb::Body body = GetUniverse().get<cqspb::Body>(player_civ->starting_planet);
    system_renderer->SeeStarSystem();

    //SeeStarSystem(GetApp(), body.star_system);
    SeePlanet(GetApp(), GetUniverse().planets["earth"]);
    //selected_planet = player_civ->starting_planet;

    AddUISystem<cqsps::SysPlanetInformation>();
    AddUISystem<cqsps::SysTurnSaveWindow>();
    AddUISystem<cqsps::SysStarSystemTree>();
    AddUISystem<cqsps::SysPauseMenu>();
    AddUISystem<cqsps::SysDebugMenu>();
    AddUISystem<cqsps::SysCommand>();
    AddUISystem<cqsps::CivilizationInfoPanel>();
    AddUISystem<cqsps::SysFieldViewer>();
    AddUISystem<cqsps::SysTechnologyProjectViewer>();
    AddUISystem<cqsps::SysTechnologyViewer>();

    AddUISystem<cqsps::gui::SysEvent>();
    simulation->tick();
}

void cqsp::scene::UniverseScene::Update(float deltaTime) {
    ZoneScoped;

    // Check for last tick
    if (GetUniverse().ToTick() && !game_halted) {
        // Game tick
        simulation->tick();
        system_renderer->OnTick();
    }

    if (!game_halted) {
        if (!ImGui::GetIO().WantCaptureKeyboard && GetApp().ButtonIsReleased(engine::KeyInput::KEY_M)) {
            view_mode = !view_mode;
        }
        system_renderer->Update(deltaTime);
        // Check to see if you have to switch
    }

    DoScreenshot();

    if (view_mode) {
        GetUniverse().clear<cqsp::client::systems::MouseOverEntity>();
        system_renderer->GetMouseOnObject(GetApp().GetMouseX(), GetApp().GetMouseY());
    }

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
    // Render star system renderer ui
    system_renderer->DoUI(deltaTime);
}

void cqsp::scene::UniverseScene::Render(float deltaTime) {
    ZoneScoped;
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render(deltaTime);
}

void cqsp::scene::UniverseScene::DoScreenshot() {
    // Take screenshot
    if ((GetApp().ButtonIsReleased(engine::KeyInput::KEY_F1) &&
            GetApp().ButtonIsHeld(engine::KeyInput::KEY_F10)) ||
        (GetApp().ButtonIsHeld(engine::KeyInput::KEY_F1) &&
            GetApp().ButtonIsReleased(engine::KeyInput::KEY_F10))) {
        GetApp().Screenshot();
    }
}

entt::entity cqsp::scene::GetCurrentViewingPlanet(cqsp::common::Universe& universe) {
    return universe.view<cqsp::client::systems::FocusedPlanet>().front();
}

void cqsp::scene::SeePlanet(cqsp::engine::Application& app, entt::entity ent) {
    app.GetUniverse().clear<cqsp::client::systems::FocusedPlanet>();
    app.GetUniverse().emplace<cqsp::client::systems::FocusedPlanet>(ent);
}

void cqsp::scene::SetGameHalted(bool b) { game_halted = b; }

bool cqsp::scene::IsGameHalted() { return game_halted; }
