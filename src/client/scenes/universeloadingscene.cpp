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
#include "client/scenes/universeloadingscene.h"

#include <spdlog/spdlog.h>

#include <string>

#include "client/components/clientctx.h"
#include "client/scenes/selection/countryselectionscene.h"
#include "client/scenes/universe/lua/hovertext.h"
#include "client/scenes/universe/universescene.h"
#include "client/systems/assetloading.h"
#include "client/systems/universeloader.h"
#include "core/components/player.h"
#include "core/scripting/functionreg.h"
#include "core/scripting/luafunctions.h"
#include "core/systems/sysuniversegenerator.h"
#include "core/util/save/savegame.h"

namespace cqsp::client::scene {

UniverseLoadingScene::UniverseLoadingScene(engine::Application& app) : ClientScene(app) {}

UniverseLoadingScene::~UniverseLoadingScene() {
    GetApp().CloseDocument("../data/core/gui/screens/universe_loading_screen.rml");
}

void UniverseLoadingScene::Init() {
    auto loading = [&]() { LoadCurrentUniverse(); };

    m_completed_loading = false;
    thread = std::make_unique<std::thread>(loading);
    document = GetApp().LoadDocument("../data/core/gui/screens/universe_loading_screen.rml");
    if (document != nullptr) {
        document->Show();
    }
}

void UniverseLoadingScene::Update(float deltaTime) {
    if (m_completed_loading && thread->joinable()) {
        CompleteLoading();
    }
}

void UniverseLoadingScene::Ui(float deltaTime) {}

void UniverseLoadingScene::Render(float deltaTime) {}

void UniverseLoadingScene::LoadCurrentUniverse() {
    LoadUniverse(GetAssetManager(), *dynamic_cast<ConquerSpace*>(GetApp().GetGame()));
    // Load saves
    if (GetUniverse().ctx().contains<ctx::GameLoad>()) {
        const std::string& load_dir = GetUniverse().ctx().at<ctx::GameLoad>().load_dir;
        SPDLOG_INFO("Loading save {}", load_dir);
        core::save::load_game(GetUniverse(), load_dir);
    }

    SPDLOG_INFO("Done loading the universe, entering game");
    m_completed_loading = true;
}

void UniverseLoadingScene::InitializeGameScene() {
    core::scripting::LoadFunctions(GetUniverse(), GetApp().GetSolState());
    client::scripting::LoadHoverFunctions(GetUniverse(), GetApp().GetSolState());
    GetApp().GetSolState()["global_state"] = GetApp().GetSolState().create_table_with('test', 0);
}

void UniverseLoadingScene::CompleteLoading() {
    // Switch scene
    thread->join();
    auto system_renderer = std::make_unique<systems::SysStarSystemRenderer>(GetUniverse(), GetApp());
    system_renderer->Initialize();
    system_renderer->SeeStarSystem();
    SeePlanet(GetUniverse(), GetUniverse().planets["earth"]);
    auto simulation = std::make_unique<core::systems::simulation::Simulation>(
        dynamic_cast<ConquerSpace*>(GetApp().GetGame())->GetGame());
    simulation->Init();
    // Init simulation tick
    simulation->tick();
    InitializeGameScene();

    if (GetApp().HasCmdLineArgs("-i")) {
        // Then we should check that we have that
        auto iterator = std::find(GetApp().GetCmdLineArgs().begin(), GetApp().GetCmdLineArgs().end(), "-i");
        ++iterator;
        if (iterator != GetApp().GetCmdLineArgs().end() && GetUniverse().countries.contains(*iterator)) {
            // Then we should check for a country and we just set our country to that
            // Then we go to universe scene
            auto player = GetUniverse().countries[*iterator];
            GetUniverse().emplace<core::components::Player>(GetUniverse().countries["usa"]);
            GetApp().SetScene<UniverseScene>(std::move(system_renderer), std::move(simulation));
            return;
        }
    }

    GetApp().SetScene<CountrySelectionScene>(std::move(system_renderer), std::move(simulation));
}
}  // namespace cqsp::client::scene