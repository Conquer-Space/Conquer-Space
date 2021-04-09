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

#include "common/universe.h"
#include "common/components/bodies.h"
#include "common/components/orbit.h"
#include "common/systems/sysuniversegenerator.h"

conquerspace::scene::UniverseLoadingScene::UniverseLoadingScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseLoadingScene::Init() {
    auto loading = [&]() {
        LoadUniverse();
    };

    thread = std::make_unique<boost::thread>(loading);
    thread->detach();
}

void conquerspace::scene::UniverseLoadingScene::Update(float deltaTime) {}

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
    conquerspace::systems::universegenerator::SysGenerateUniverse(GetApplication());
}
