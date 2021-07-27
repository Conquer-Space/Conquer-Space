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

#include <sol/sol.hpp>

#include "client/scenes/universescene.h"
#include "common/universe.h"
#include "common/components/bodies.h"
#include "common/components/resource.h"
#include "common/components/name.h"
#include "common/components/movement.h"
#include "common/systems/sysuniversegenerator.h"

conquerspace::scene::UniverseLoadingScene::UniverseLoadingScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseLoadingScene::Init() {
    auto loading = [&]() {
        LoadUniverse();
    };

    m_completed_loading = false;
    thread = std::make_unique<std::thread>(loading);
    thread->detach();
}

void conquerspace::scene::UniverseLoadingScene::Update(float deltaTime) {
    if (m_completed_loading) {
        // Switch scene
        GetApp().SetScene<conquerspace::scene::UniverseScene>();
    }
}

void conquerspace::scene::UniverseLoadingScene::Ui(float deltaTime) {
    ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Conquer Space", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Loading...");
    ImGui::ProgressBar(0 / 100.f);
    ImGui::End();
}

void conquerspace::scene::UniverseLoadingScene::Render(float deltaTime) {}

void LoadGoods(conquerspace::engine::Application& app) {
    namespace cqspc = conquerspace::common::components;
    conquerspace::asset::HjsonAsset* good_assets = app
                .GetAssetManager().GetAsset<conquerspace::asset::HjsonAsset>("goods");
    int assets_loaded = 0;
    for (int i = 0; i < good_assets->data.size(); i++) {
        Hjson::Value val = good_assets->data[i];
        // Create good
        entt::entity good = app.GetUniverse().create();
        auto& good_object = app.GetUniverse().emplace<cqspc::Good>(good);
        good_object.mass = val["mass"];
        good_object.volume = val["volume"];
        auto &name_object = app.GetUniverse().emplace<cqspc::Name>(good);
        name_object.name = val["name"].to_string();
        auto &id_object = app.GetUniverse().emplace<cqspc::Identifier>(good);
        id_object.identifier = val["identifier"].to_string();
        app.GetUniverse().goods[val["identifier"].to_string()] = good;
        assets_loaded++;
    }
    SPDLOG_INFO("Loaded {} goods", assets_loaded);
}

void LoadRecipes(conquerspace::engine::Application& app) {
    namespace cqspc = conquerspace::common::components;

    conquerspace::asset::HjsonAsset* recipe_asset = app
                .GetAssetManager().GetAsset<conquerspace::asset::HjsonAsset>("recipes");
    for (int i = 0; i < recipe_asset->data.size(); i++) {
        Hjson::Value& val = recipe_asset->data[i];

        entt::entity recipe = app.GetUniverse().create();
        auto& recipe_component = app.GetUniverse().emplace<cqspc::Recipe>(recipe);
        Hjson::Value input_value = val["input"];
        for (auto input_good : input_value) {
            recipe_component.input[app.GetUniverse().goods[input_good.first]] =
                input_good.second;
        }

        Hjson::Value output_value = val["output"];
        for (auto output_good : output_value) {
            recipe_component.output[app.GetUniverse().goods[output_good.first]] =
                output_good.second;
        }

        auto &name_object = app.GetUniverse().emplace<cqspc::Identifier>(recipe);
        name_object.identifier = val["identifier"].to_string();
        app.GetUniverse().recipes[name_object.identifier] = recipe;
    }
}

void conquerspace::scene::UniverseLoadingScene::LoadUniverse() {
    namespace cqspa = conquerspace::asset;
    // Load goods
    LoadGoods(GetApp());
    LoadRecipes(GetApp());

    // Load universe
    conquerspace::common::systems::universegenerator::ScriptUniverseGenerator
                                                    script_generator(GetApp().GetScriptInterface());

    script_generator.Generate(GetApp().GetUniverse());
    m_completed_loading = true;
}
