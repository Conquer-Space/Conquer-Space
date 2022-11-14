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
#include "client/scenes/universeloadingscene.h"

#include <spdlog/spdlog.h>

#include <string>

#include "client/scenes/universescene.h"
#include "client/systems/assetloading.h"
#include "common/systems/sysuniversegenerator.h"

cqsp::scene::UniverseLoadingScene::UniverseLoadingScene(cqsp::engine::Application& app) : Scene(app) {}

cqsp::scene::UniverseLoadingScene::~UniverseLoadingScene() {
    GetApp().CloseDocument("../data/core/gui/screens/universe_loading_screen.rml");
}

void cqsp::scene::UniverseLoadingScene::Init() {
    auto loading = [&]() { LoadUniverse(); };

    m_completed_loading = false;
    thread = std::make_unique<std::thread>(loading);
    document = GetApp().LoadDocument("../data/core/gui/screens/universe_loading_screen.rml");
    if (document) {
        document->Show();
    }
}

void cqsp::scene::UniverseLoadingScene::Update(float deltaTime) {
    if (m_completed_loading && thread->joinable()) {
        // Switch scene
        thread->join();
        GetApp().SetScene<cqsp::scene::UniverseScene>();
    }
}

void cqsp::scene::UniverseLoadingScene::Ui(float deltaTime) {}

void cqsp::scene::UniverseLoadingScene::Render(float deltaTime) {}

void cqsp::scene::UniverseLoadingScene::LoadUniverse() {
    cqsp::client::systems::LoadAllResources(GetApp());
    SPDLOG_INFO("Made all game resources into game objects");
    using cqsp::asset::TextAsset;
    // Process scripts for core
    TextAsset* script_list = GetAssetManager().GetAsset<TextAsset>("core:base");
    GetApp().GetScriptInterface().RunScript(script_list->data);
    SPDLOG_INFO("Done loading scripts");
    using cqsp::common::systems::universegenerator::ScriptUniverseGenerator;
    // Load universe
    ScriptUniverseGenerator script_generator(GetApp().GetScriptInterface());

    script_generator.Generate(GetUniverse());
    SPDLOG_INFO("Done loading the universe, entering game");
    m_completed_loading = true;
}
