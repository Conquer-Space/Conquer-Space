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
#include "client/scenes/loadingscene.h"

#include <spdlog/spdlog.h>

#include <fstream>
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/assetmanager.h"
#include "client/scenes/mainmenuscene.h"
#include "engine/gui.h"
#include "common/scripting/scripting.h"

cqsp::scene::LoadingScene::LoadingScene(
    cqsp::engine::Application& app)
    : cqsp::engine::Scene(app) {
    m_done_loading = false;
    percentage = 0;
}

void cqsp::scene::LoadingScene::Init() {
    auto loading = [&]() {
        SPDLOG_INFO("Loading resources");
        LoadResources();
        SPDLOG_INFO("Need halt: {}", need_halt);
    };

    thread = std::make_unique<std::thread>(loading);
    thread->detach();
}

void cqsp::scene::LoadingScene::Update(float deltaTime) {
    while (assetLoader.QueueHasItems()) {
        assetLoader.BuildNextAsset();
    }
    if (m_done_loading && !assetLoader.QueueHasItems() && !need_halt) {
        // Load font after all the shaders are done
        LoadFont();

        // Load audio
        auto hjson = GetApp().GetAssetManager().GetAsset<cqsp::asset::HjsonAsset>("core:ui_sounds");
        for (auto element : hjson->data) {
            auto audio_asset = GetApp().GetAssetManager().GetAsset<cqsp::asset::AudioAsset>(element.second);
            GetApp().GetAudioInterface().AddAudioClip(element.first, audio_asset);
        }
        // Set main menu scene
        GetApp().SetScene<cqsp::scene::MainMenuScene>();
    }
}

void cqsp::scene::LoadingScene::Ui(float deltaTime) {
    ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Conquer Space", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Loading...");
    ImGui::ProgressBar(percentage/100.f);
    ImGui::End();

    if (m_done_loading && need_halt) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                       ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Error", NULL, ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Found missing assets!");

        if (ImGui::Button("Exit", ImVec2(-FLT_MIN, 0))) {
            GetApp().ExitApplication();
        }
        ImGui::End();
    }
}

void cqsp::scene::LoadingScene::Render(float deltaTime) { }

void cqsp::scene::LoadingScene::LoadResources() {
    // Loading goes here
    std::ifstream assetLibrary("../data/core/assets.hjson");

    assetLoader.manager = &GetApp().GetAssetManager();
    assetLoader.LoadAssets(assetLibrary);

    SPDLOG_INFO("Done loading items");
    need_halt = !assetLoader.GetMissingAssets().empty();
    m_done_loading = true;
}

void cqsp::scene::LoadingScene::LoadFont() {
    cqsp::asset::ShaderProgram* fontshader = new asset::ShaderProgram(*GetApp().GetAssetManager()
        .GetAsset<cqsp::asset::Shader>("core:fontvertexshader"),
        *GetApp()
        .GetAssetManager()
        .GetAsset<cqsp::asset::Shader>("core:fontfragshader"));

    cqsp::asset::Font* font = GetApp()
        .GetAssetManager()
        .GetAsset<cqsp::asset::Font>("core:defaultfont");

    glm::mat4 projection =
        glm::ortho(0.0f, static_cast<float>(GetApp().GetWindowWidth()), 0.0f,
                    static_cast<float>(GetApp().GetWindowHeight()));

    fontshader->UseProgram();
    fontshader->setMat4("projection", projection);

    GetApp().SetFont(font);
    GetApp().SetFontShader(fontshader);
}
