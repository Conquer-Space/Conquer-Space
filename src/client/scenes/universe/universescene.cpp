/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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

#include <RmlUi/Core/Factory.h>

#include <cmath>
#include <string>

#include "client/components/clientctx.h"
#include "client/scenes/objecteditor/sysfieldviewer.h"
#include "client/scenes/universe/interface/debug/sysdebuggui.h"
#include "client/scenes/universe/interface/imguiinterface.h"
#include "client/scenes/universe/interface/launchvehiclewindow.h"
#include "client/scenes/universe/interface/maintooltip.h"
#include "client/scenes/universe/interface/mapmodewindow.h"
#include "client/scenes/universe/interface/orbitfilter.h"
#include "client/scenes/universe/interface/provincewindow.h"
#include "client/scenes/universe/interface/rightclickwindow.h"
#include "client/scenes/universe/interface/spaceshipwindow.h"
#include "client/scenes/universe/interface/sysevent.h"
#include "client/scenes/universe/interface/syspausemenu.h"
#include "client/scenes/universe/interface/sysplanetmarketinformation.h"
#include "client/scenes/universe/interface/sysstarsystemtree.h"
#include "client/scenes/universe/interface/systechviewer.h"
#include "client/scenes/universe/interface/turnsavewindow.h"
#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/player.h"
#include "core/components/population.h"
#include "core/components/resource.h"
#include "core/components/surface.h"
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
#include "universescene.h"

// If the game is paused or not, like when escape is pressed
bool game_halted = false;

namespace cqsp::client::scene {

namespace components = core::components;
namespace bodies = components::bodies;
namespace systems = client::systems;

using core::systems::simulation::Simulation;

UniverseScene::UniverseScene(engine::Application& app) : ClientScene(app) {}

void UniverseScene::Init() {
    ZoneScoped;

    simulation = std::make_unique<Simulation>(dynamic_cast<ConquerSpace*>(GetApp().GetGame())->GetGame());

    system_renderer = std::make_unique<systems::SysStarSystemRenderer>(GetUniverse(), GetApp());
    system_renderer->Initialize();

    GetUniverse().ctx().emplace<client::ctx::PauseOptions>();
    GetUniverse().ctx().emplace<client::ctx::HoveringItem>();
    GetUniverse().ctx().emplace<client::ctx::MapMode>(client::ctx::MapMode::NoMapMode);

    system_renderer->SeeStarSystem();

    SeePlanet(GetUniverse(), GetUniverse().planets["earth"]);

    AddUISystem<systems::SysStarSystemTree>();
    AddUISystem<systems::SysPauseMenu>();
    AddUISystem<systems::SysDebugMenu>();

    AddUISystem<systems::SpaceshipWindow>();
    AddUISystem<systems::SysProvinceInformation>();
    AddUISystem<systems::SysOrbitFilter>();
    AddUISystem<systems::ImGuiInterface>();
    AddUISystem<systems::MapModeWindow>();
    AddUISystem<systems::LaunchVehicleWindow>();

    AddUISystem<systems::gui::SysEvent>();
    simulation->Init();
    simulation->tick();  // Why do we tick the simulation once here? Idk

    AddRmlUiSystem<systems::rmlui::TurnSaveWindow>();
    AddRmlUiSystem<systems::rmlui::RightClickWindow>();
    AddRmlUiSystem<systems::rmlui::ToolTipWindow>();
}

void UniverseScene::Update(float deltaTime) {
    ZoneScoped;
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (GetApp().ButtonIsReleased(engine::KeyInput::KEY_SPACE)) {
            ToggleTick();
        }
    }

    CheckUiReload();
    ManageTick();

    if (!game_halted) {
        system_renderer->Update(deltaTime);
        // Check to see if you have to switch
    }

    DoScreenshot();

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

void UniverseScene::Ui(float deltaTime) {
    for (auto& ui : user_interfaces) {
        ui->DoUI(deltaTime);
    }
    system_renderer->DoUI(deltaTime);
}

void UniverseScene::Render(float deltaTime) {
    ZoneScoped;
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render(deltaTime);
}

void UniverseScene::DoScreenshot() {
    // Take screenshot
    if ((GetApp().ButtonIsReleased(engine::KeyInput::KEY_F1) && GetApp().ButtonIsHeld(engine::KeyInput::KEY_F10)) ||
        (GetApp().ButtonIsHeld(engine::KeyInput::KEY_F1) && GetApp().ButtonIsReleased(engine::KeyInput::KEY_F10))) {
        GetApp().Screenshot();
    }
}

void UniverseScene::CheckUiReload() {
    if ((GetApp().ButtonIsReleased(engine::KeyInput::KEY_F5))) {
        Rml::Factory::ClearStyleSheetCache();
        for (auto& ui : documents) {
            ui->ReloadWindow();
        }
    }
}

void UniverseScene::ManageTick() {
    auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
    int tick_speed = ctx::tick_speeds[pause_opt.tick_speed];
    double tick_length = static_cast<float>(tick_speed) / 1000.f;
    if (tick_speed < 0) {
        tick_length = 1 / 1000.f;
    }
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
        if (ctx::tick_speeds[pause_opt.tick_speed] < 0) {
            for (int i = 0; i < -ctx::tick_speeds[pause_opt.tick_speed]; i++) {
                simulation->tick();
            }
        } else {
            simulation->tick();
        }
        system_renderer->OnTick();
    }
}

void UniverseScene::ToggleTick() {
    auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
    pause_opt.to_tick = !pause_opt.to_tick;
}

entt::entity GetCurrentViewingPlanet(core::Universe& universe) {
    return universe.view<systems::FocusedPlanet>().front();
}

void SeePlanet(core::Universe& universe, entt::entity ent) {
    universe.clear<systems::FocusedPlanet>();
    universe.emplace<systems::FocusedPlanet>(ent);
}

void SetGameHalted(bool b) { game_halted = b; }

bool IsGameHalted() { return game_halted; }

}  // namespace cqsp::client::scene