/*
 * Copyright 2021 Conquer Space
 */
#include "client/scenes/universeloadingscene.h"

#include <spdlog/spdlog.h>

#include <boost/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/normal_distribution.hpp>

#include <sol/sol.hpp>

#include "client/scenes/universescene.h"
#include "common/universe.h"
#include "common/components/bodies.h"
#include "common/components/resource.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/systems/sysuniversegenerator.h"

conquerspace::scene::UniverseLoadingScene::UniverseLoadingScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseLoadingScene::Init() {
    auto loading = [&]() {
        LoadUniverse();
    };

    m_completed_loading = false;
    thread = std::make_unique<boost::thread>(loading);
    thread->detach();
}

void conquerspace::scene::UniverseLoadingScene::Update(float deltaTime) {
    if (m_completed_loading) {
        // Switch scene
        GetApplication().SetScene<conquerspace::scene::UniverseScene>();
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

void conquerspace::scene::UniverseLoadingScene::LoadUniverse() {
    // Load goods
    conquerspace::asset::HjsonAsset* good_assets = GetApplication()
                .GetAssetManager().GetAsset<conquerspace::asset::HjsonAsset>("goods");
    int assets_loaded = 0;
    for (int i = 0; i < good_assets->data.size(); i++) {
        Hjson::Value& val = good_assets->data[i];
        // Create good
        entt::entity good = GetApplication().GetUniverse().registry.create();
        auto& good_object = GetApplication()
            .GetUniverse()
            .registry.emplace<conquerspace::components::Good>(good);
        good_object.mass = val["mass"];
        good_object.volume = val["volume"];
        auto &name_object =
            GetApplication()
                .GetUniverse()
                .registry.emplace<conquerspace::components::Name>(good);
        name_object.name = val["name"].to_string();
        auto &id_object =
            GetApplication()
                .GetUniverse()
                .registry.emplace<conquerspace::components::Identifier>(good);
        id_object.identifier = val["identifier"].to_string();
        assets_loaded++;
    }
    spdlog::info("Loaded {} goods", assets_loaded);

    conquerspace::systems::universegenerator::SysGenerateUniverse(GetApplication());
    m_completed_loading = true;
}
