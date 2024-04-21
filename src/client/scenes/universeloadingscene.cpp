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
#include "client/scenes/universeloadingscene.h"

#include <spdlog/spdlog.h>

#include <string>

#include "client/components/clientctx.h"
#include "client/scenes/universe/universescene.h"
#include "client/systems/assetloading.h"
#include "client/systems/savegame.h"
#include "common/systems/sysuniversegenerator.h"

using cqsp::scene::UniverseLoadingScene;

UniverseLoadingScene::UniverseLoadingScene(cqsp::engine::Application& app) : cqsp::client::Scene(app) {}

UniverseLoadingScene::~UniverseLoadingScene() {
    GetApp().CloseDocument("../data/core/gui/screens/universe_loading_screen.rml");
}

void UniverseLoadingScene::Init() {
    auto loading = [&]() { LoadUniverse(); };

    m_completed_loading = false;
    thread = std::make_unique<std::thread>(loading);
    document = GetApp().LoadDocument("../data/core/gui/screens/universe_loading_screen.rml");
    if (document != nullptr) {
        document->Show();
    }
}

void UniverseLoadingScene::Update(float deltaTime) {
    if (m_completed_loading && thread->joinable()) {
        // Switch scene
        thread->join();
        GetApp().SetScene<cqsp::scene::UniverseScene>();
    }
}

void UniverseLoadingScene::Ui(float deltaTime) {}

void UniverseLoadingScene::Render(float deltaTime) {}

void UniverseLoadingScene::LoadUniverse() {
    client::systems::LoadAllResources(GetApp(), *dynamic_cast<cqsp::client::ConquerSpace*>(GetApp().GetGame()));
    SPDLOG_INFO("Made all game resources into game objects");
    using asset::TextAsset;
    // Process scripts for core
    TextAsset* script_list = GetAssetManager().GetAsset<TextAsset>("core:base");
    GetScriptInterface().RunScript(script_list->data);
    SPDLOG_INFO("Done loading scripts");
    using common::systems::universegenerator::ScriptUniverseGenerator;
    // Load universe
    ScriptUniverseGenerator script_generator(GetScriptInterface());

    script_generator.Generate(GetUniverse());
    if (GetUniverse().ctx().contains<client::ctx::GameLoad>()) {
        const std::string& load_dir = GetUniverse().ctx().at<client::ctx::GameLoad>().load_dir;
        SPDLOG_INFO("Loading save {}", load_dir);
        client::save::load_game(GetUniverse(), load_dir);
    }

    SPDLOG_INFO("Done loading the universe, entering game");
    m_completed_loading = true;
}
