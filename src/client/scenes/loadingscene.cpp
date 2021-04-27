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

conquerspace::scene::LoadingScene::LoadingScene(
    conquerspace::engine::Application& app)
    : conquerspace::engine::Scene(app) {
    m_done_loading = false;
    percentage = 0;
}

void conquerspace::scene::LoadingScene::Init() {
    auto loading = [&]() {
        spdlog::info("Loading resources");
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
        // Done!

        // Load font after all the shaders are done

        LoadFont();
        GetApplication().SetScene<conquerspace::scene::MainMenuScene>();
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
    // Load i18n
    std::ifstream ifs("../data/core/i18n/default.po");
    std::string po_file{std::istreambuf_iterator<char>{ifs},
                        std::istreambuf_iterator<char>()};
    auto a = std::make_unique<spirit_po::default_catalog>(
        spirit_po::default_catalog::from_range(po_file));

    std::ifstream assetLibrary("../data/core/assets.hjson");

    assetLoader.manager = &GetApplication().GetAssetManager();
    assetLoader.LoadAssets(assetLibrary);

    spdlog::info("Done loading items");
    m_done_loading = true;
}

void conquerspace::scene::LoadingScene::LoadFont() {
    conquerspace::asset::ShaderProgram* fontshader = new asset::ShaderProgram(*GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Shader>("fontvertexshader"),
        *GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Shader>("fontfragshader"));

    conquerspace::asset::Font* font = GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Font>("defaultfont");

    glm::mat4 projection =
        glm::ortho(0.0f, static_cast<float>(GetApplication().GetWindowWidth()), 0.0f,
                    static_cast<float>(GetApplication().GetWindowHeight()));

    fontshader->UseProgram();
    fontshader->setMat4("projection", projection);

    GetApplication().SetFont(font);
    GetApplication().SetFontShader(fontshader);
}
