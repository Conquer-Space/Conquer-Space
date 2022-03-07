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
#include "client/systems/clientscripting.h"
#include "common/systems/sysuniversegenerator.h"
#include "common/scripting/luafunctions.h"

#include "common/systems/loading/loadgoods.h"
#include "common/systems/loading/loadnames.h"

cqsp::scene::UniverseLoadingScene::UniverseLoadingScene(cqsp::engine::Application& app) : Scene(app) {}

void cqsp::scene::UniverseLoadingScene::Init() {
    auto loading = [&]() {
        LoadUniverse();
    };

    m_completed_loading = false;
    thread = std::make_unique<std::thread>(loading);
    thread->detach();
}

void cqsp::scene::UniverseLoadingScene::Update(float deltaTime) {
    if (m_completed_loading) {
        // Switch scene
        GetApp().SetScene<cqsp::scene::UniverseScene>();
    }
}

void cqsp::scene::UniverseLoadingScene::Ui(float deltaTime) {
    ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.8, -1), ImGuiCond_Always);
    ImGui::Begin("Conquer Space", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Loading...");
    ImGui::ProgressBar(0 / 100.f);
    ImGui::End();
}

void cqsp::scene::UniverseLoadingScene::Render(float deltaTime) {}

void LoadResource(cqsp::engine::Application& app, std::string asset_name,
                    void (*func)(cqsp::common::Universe& universe, Hjson::Value& recipes)) {
    namespace cqspc = cqsp::common::components;
    for (auto it = app.GetAssetManager().GetPackageBegin(); it != app.GetAssetManager().GetPackageEnd(); it++) {
        if (!it->second->HasAsset(asset_name)) {
            continue;
        }
        cqsp::asset::HjsonAsset* good_assets = it->second->GetAsset<cqsp::asset::HjsonAsset>(asset_name);
        try {
            func(app.GetUniverse(), good_assets->data);
        } catch (std::runtime_error& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: {}", asset_name, error.what());
        } catch (Hjson::index_out_of_bounds &) {
        }
    }
}

void cqsp::scene::UniverseLoadingScene::LoadUniverse() {
    namespace cqspa = cqsp::asset;
    namespace cqspc = cqsp::common::components;

    LoadResource(GetApp(), "goods", cqsp::common::systems::loading::LoadGoods);
    LoadResource(GetApp(), "recipes", cqsp::common::systems::loading::LoadRecipes);
    LoadResource(GetApp(), "names", cqsp::common::systems::loading::LoadNameLists);
    // Initialize planet terrains
    cqsp::asset::HjsonAsset* asset = GetAssetManager().GetAsset<cqsp::asset::HjsonAsset>("core:terrain_colors");
    cqsp::common::systems::loading::LoadTerrainData(GetApp().GetUniverse(), asset->data);

    // Load scripts
    // Load lua functions
    cqsp::scripting::LoadFunctions(GetApp().GetUniverse(),
                                   GetApp().GetScriptInterface());
    cqsp::client::scripting::ClientFunctions(GetApp());

    // Load universe
    // Register data groups
    auto& script_interface = GetApp().GetScriptInterface();
    script_interface.RegisterDataGroup("generators");
    script_interface.RegisterDataGroup("events");

    using cqsp::asset::TextAsset;
    // Process scripts for core
    TextAsset* script_list = GetAssetManager().GetAsset<TextAsset>("core:base");
    script_interface.RunScript(script_list->data);

    using cqsp::common::systems::universegenerator::ScriptUniverseGenerator;
    // Load universe
    ScriptUniverseGenerator script_generator(script_interface);

    script_generator.Generate(GetUniverse());
    m_completed_loading = true;
}
