/*
 * Copyright 2021 Conquer Space
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

conquerspace::scene::LoadingScene::LoadingScene(
    conquerspace::engine::Application& app)
    : conquerspace::engine::Scene(app) {
    m_done_loading = false;
    percentage = 0;
}

void conquerspace::scene::LoadingScene::Init() {
    auto loading = [&]() {
        SPDLOG_INFO("Loading resources");
        LoadResources();
    };

    thread = std::make_unique<boost::thread>(loading);
    thread->detach();
}

void conquerspace::scene::LoadingScene::Update(float deltaTime) {
    while (assetLoader.QueueHasItems()) {
        assetLoader.BuildNextAsset();
    }
    if (m_done_loading && !assetLoader.QueueHasItems()) {
        // Load font after all the shaders are done
        LoadFont();
        // Set main menu scene
        GetApp().SetScene<conquerspace::scene::MainMenuScene>();
    }
}

void conquerspace::scene::LoadingScene::Ui(float deltaTime) {
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
}

void conquerspace::scene::LoadingScene::Render(float deltaTime) { }

void conquerspace::scene::LoadingScene::LoadResources() {
    // Loading goes here
    std::ifstream assetLibrary("../data/core/assets.hjson");

    assetLoader.manager = &GetApp().GetAssetManager();
    assetLoader.LoadAssets(assetLibrary);

    GetApp().GetScriptInterface().RegisterDataGroup("generators");
    GetApp().GetScriptInterface().RunScript(GetApp().GetAssetManager().
                                    GetAsset<conquerspace::asset::TextAsset>("defaultgen")->data);

    // Process entirity of directories
    GetApp().GetScriptInterface().RegisterDataGroup("events");

    conquerspace::asset::TextDirectoryAsset* event_list = GetApp().GetAssetManager()
                            .GetAsset<conquerspace::asset::TextDirectoryAsset>("defaultevent");
    for (auto& text : event_list->data) {
        GetApp().GetScriptInterface().RunScript(text);
    }
    SPDLOG_INFO("Done loading items");
    m_done_loading = true;
}

void conquerspace::scene::LoadingScene::LoadFont() {
    conquerspace::asset::ShaderProgram* fontshader = new asset::ShaderProgram(*GetApp()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Shader>("fontvertexshader"),
        *GetApp()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Shader>("fontfragshader"));

    conquerspace::asset::Font* font = GetApp()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Font>("defaultfont");

    glm::mat4 projection =
        glm::ortho(0.0f, static_cast<float>(GetApp().GetWindowWidth()), 0.0f,
                    static_cast<float>(GetApp().GetWindowHeight()));

    fontshader->UseProgram();
    fontshader->setMat4("projection", projection);

    GetApp().SetFont(font);
    GetApp().SetFontShader(fontshader);
}
