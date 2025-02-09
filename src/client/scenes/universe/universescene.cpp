/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "client/scenes/universe/universescene.h"

#include <cmath>
#include <string>

#include "client/components/clientctx.h"
#include "client/scenes/objecteditor/sysfieldviewer.h"
#include "client/scenes/universe/interface/civilizationinfopanel.h"
#include "client/scenes/universe/interface/debug/sysdebuggui.h"
#include "client/scenes/universe/interface/marketwindow.h"
#include "client/scenes/universe/interface/orbitfilter.h"
#include "client/scenes/universe/interface/provincewindow.h"
#include "client/scenes/universe/interface/spaceshipwindow.h"
#include "client/scenes/universe/interface/sysevent.h"
#include "client/scenes/universe/interface/syspausemenu.h"
#include "client/scenes/universe/interface/sysstarsystemtree.h"
#include "client/scenes/universe/interface/systechviewer.h"
#include "client/scenes/universe/interface/systurnsavewindow.h"
#include "client/scenes/universe/interface/turnsavewindow.h"
#include "client/systems/syscommand.h"
#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/surface.h"
#include "engine/graphics/primitives/cube.h"
#include "engine/graphics/primitives/polygon.h"
#include "engine/graphics/primitives/uvsphere.h"
#include "engine/gui.h"
#include "engine/renderer/renderer.h"
#include "fmt/format.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/polar_coordinates.hpp"
#include "tracy/Tracy.hpp"

// If the game is paused or not, like when escape is pressed
bool game_halted = false;

cqsp::scene::UniverseScene::UniverseScene(cqsp::engine::Application& app) : cqsp::client::Scene(app) {}

void cqsp::scene::UniverseScene::Init() {
    ZoneScoped;
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspco = cqsp::common;
    namespace cqspc = cqsp::common::components;
    namespace cqsps = cqsp::client::systems;

    using cqspco::systems::simulation::Simulation;
    simulation = std::make_unique<Simulation>(dynamic_cast<cqsp::client::ConquerSpace*>(GetApp().GetGame())->GetGame());

    system_renderer = new cqsps::SysStarSystemRenderer(GetUniverse(), GetApp());
    system_renderer->Initialize();

    GetUniverse().ctx().emplace<client::ctx::PauseOptions>();

    system_renderer->SeeStarSystem();

    SeePlanet(GetUniverse(), GetUniverse().planets["earth"]);

    //AddUISystem<cqsps::SysTurnSaveWindow>();
    AddUISystem<cqsps::SysStarSystemTree>();
    AddUISystem<cqsps::SysPauseMenu>();
    AddUISystem<cqsps::SysDebugMenu>();
    //AddUISystem<cqsps::SysCommand>();
    AddUISystem<cqsps::CivilizationInfoPanel>();
    AddUISystem<cqsps::SpaceshipWindow>();
    //AddUISystem<cqsps::SysFieldViewer>();
    //AddUISystem<cqsps::SysTechnologyProjectViewer>();
    //AddUISystem<cqsps::SysTechnologyViewer>();
    AddUISystem<cqsps::SysProvinceInformation>();
    AddUISystem<cqsps::SysOrbitFilter>();
    //AddUISystem<cqsps::SysPlanetMarketInformation>();

    AddUISystem<cqsps::gui::SysEvent>();
    simulation->tick();

    AddRmlUiSystem<cqsps::rmlui::TurnSaveWindow>();
}

void cqsp::scene::UniverseScene::Update(float deltaTime) {
    ZoneScoped;

    auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (GetApp().ButtonIsReleased(engine::KeyInput::KEY_SPACE)) {
            ToggleTick();
        }
    }

    double tick_length = static_cast<float>(tick_speeds[pause_opt.tick_speed]) / 1000.f;
    if (pause_opt.to_tick && GetApp().GetTime() - last_tick > tick_length) {
        GetUniverse().EnableTick();
        last_tick = GetApp().GetTime();
    }

    if (pause_opt.to_tick) {
        GetUniverse().tick_fraction = (GetApp().GetTime() - last_tick) / tick_length;
        if (!interp) GetUniverse().tick_fraction = 0;
    }

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

    for (auto& ui : documents) {
        ui->Update(deltaTime);
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
    system_renderer->DoUI(deltaTime);
}

void cqsp::scene::UniverseScene::Render(float deltaTime) {
    ZoneScoped;
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render(deltaTime);
}

void cqsp::scene::UniverseScene::DoScreenshot() {
    // Take screenshot
    if ((GetApp().ButtonIsReleased(engine::KeyInput::KEY_F1) && GetApp().ButtonIsHeld(engine::KeyInput::KEY_F10)) ||
        (GetApp().ButtonIsHeld(engine::KeyInput::KEY_F1) && GetApp().ButtonIsReleased(engine::KeyInput::KEY_F10))) {
        GetApp().Screenshot();
    }
}

void cqsp::scene::UniverseScene::ToggleTick() {
    auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
    pause_opt.to_tick = !pause_opt.to_tick;
}

entt::entity cqsp::scene::GetCurrentViewingPlanet(cqsp::common::Universe& universe) {
    return universe.view<cqsp::client::systems::FocusedPlanet>().front();
}

void cqsp::scene::SeePlanet(cqsp::common::Universe& universe, entt::entity ent) {
    universe.clear<cqsp::client::systems::FocusedPlanet>();
    universe.emplace<cqsp::client::systems::FocusedPlanet>(ent);
}

void cqsp::scene::SetGameHalted(bool b) { game_halted = b; }

bool cqsp::scene::IsGameHalted() { return game_halted; }
