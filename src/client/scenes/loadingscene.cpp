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
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/asset/assetmanager.h"
#include "client/scenes/mainmenuscene.h"
#include "client/scenes/universeloadingscene.h"
#include "client/scenes/texttestscene.h"
#include "engine/gui.h"
#include "common/scripting/scripting.h"
#include "common/util/paths.h"

cqsp::scene::LoadingScene::LoadingScene(cqsp::engine::Application& app)
    : cqsp::engine::Scene(app) {
    m_done_loading = false;
    percentage = 0;
}

void cqsp::scene::LoadingScene::Init() {
    auto loading = [&]() {
        SPDLOG_INFO("Loading resources");
        LoadResources();
        SPDLOG_INFO("Need halt: {}", need_halt);
        std::this_thread::sleep_for(std::chrono::milliseconds(20000) );
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
        auto hjson = GetAssetManager().GetAsset<cqsp::asset::HjsonAsset>("core:ui_sounds");
        for (auto element : hjson->data) {
            auto audio_asset = GetAssetManager().GetAsset<cqsp::asset::AudioAsset>(element.second.to_string());
            GetApp().GetAudioInterface().AddAudioClip(element.first, audio_asset);
        }
        // Set main menu scene
        if (std::find(GetApp().GetCmdLineArgs().begin(), GetApp().GetCmdLineArgs().end(), "-i")
                                                            != GetApp().GetCmdLineArgs().end()) {
            GetApp().SetScene<cqsp::scene::UniverseLoadingScene>();
        } else if (std::find(GetApp().GetCmdLineArgs().begin(), GetApp().GetCmdLineArgs().end(), "-tt")
                                                            != GetApp().GetCmdLineArgs().end()) {
            GetApp().SetScene<cqsp::scene::TextTestScene>();
        } else {
            GetApp().SetScene<cqsp::scene::MainMenuScene>();
        }
    }
}

void cqsp::scene::LoadingScene::Ui(float deltaTime) {
    ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 0), ImGuiCond_Always);
    ImGui::Begin("Conquer Space", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Loading %c", "|/-\\"[static_cast<int>(ImGui::GetTime() / 0.75f) & 3]);
    float current = static_cast<float>(assetLoader.getCurrentLoading());
    float max = static_cast<float>(assetLoader.getMaxLoading());
    std::string progress = fmt::format("{}/{}", current, max);
    ImGui::ProgressBar(current/max, ImVec2(-FLT_MIN, 0), progress.c_str());
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
    // Read core mod
    assetLoader.manager = &GetAssetManager();
    assetLoader.LoadAssets();

    SPDLOG_INFO("Done loading items");
    need_halt = !assetLoader.GetMissingAssets().empty();
    m_done_loading = true;
}

void cqsp::scene::LoadingScene::LoadFont() {
    cqsp::asset::ShaderProgram* fontshader = new asset::ShaderProgram(*GetAssetManager()
        .GetAsset<cqsp::asset::Shader>("core:fontvertexshader"),
        *GetApp()
        .GetAssetManager()
        .GetAsset<cqsp::asset::Shader>("core:fontfragshader"));

    cqsp::asset::Font* font =
        GetApp().GetAssetManager().GetAsset<cqsp::asset::Font>("core:defaultfont");

    GetApp().SetFont(font);
    GetApp().SetFontShader(fontshader);
}
