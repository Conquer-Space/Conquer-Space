/*
* Copyright 2021 Conquer Space
*/
#include "client/scenes/mainmenuscene.h"

#include <spdlog/spdlog.h>
#include <imgui_markdown.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/shader.h"
#include "engine/renderer/renderer.h"
#include "client/scenes/universeloadingscene.h"
#include "engine/asset.h"
#include "engine/gui.h"
#include "engine/renderer/text.h"

conquerspace::scene::MainMenuScene::MainMenuScene(
    conquerspace::engine::Application& app)
    : conquerspace::engine::Scene(app) { }

void conquerspace::scene::MainMenuScene::Init() {
    m_texture = GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("cqspbanner");

    m_credits = GetApplication()
                    .GetAssetManager()
                    .GetAsset<conquerspace::asset::TextAsset>("credits");

    conquerspace::primitive::MakeTexturedPaneMesh(splashscreen);
    conquerspace::primitive::MakeTexturedPaneMesh(titleBanner);

    /*rend.shaderProgram = conquerspace::engine::MakeShaderProgram(GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::ShaderAsset>("shader.pane.vert")->id,
        GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::ShaderAsset>("shader.texturedpane.frag")->id);*/

    conquerspace::asset::Texture* earthrise_texture =
        GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earthrise");

    conquerspace::asset::Texture* asset2 =
        GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("title");

    int textureid = earthrise_texture->id;
    ratio =
        static_cast<float>(GetApplication().GetWindowWidth())
            /static_cast<float>(GetApplication().GetWindowHeight());
    // Set texture

    ratio2 = static_cast<float>(asset2->height)
                        /static_cast<float>(asset2->width);

    // Create new shader program
    asset::ShaderProgram* program = new asset::ShaderProgram(*GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Shader>("shader.pane.vert"),
        *GetApplication()
        .GetAssetManager()
        .GetAsset<conquerspace::asset::Shader>("shader.texturedpane.frag"));

    splashscreen.shaderProgram = program;
    splashscreen.textures.push_back(earthrise_texture);

    titleBanner.shaderProgram = program;
    titleBanner.textures.push_back(asset2);

    // Make shaders
    splashscreen.shaderProgram->UseProgram();
    splashscreen.shaderProgram->setInt("texture1", 0);
    titleBanner.shaderProgram->UseProgram();
    splashscreen.shaderProgram->setInt("texture1", 0);
}

void conquerspace::scene::MainMenuScene::Update(float deltaTime) {}

void conquerspace::scene::MainMenuScene::Ui(float deltaTime) {
    float winWidth = width;
    float winHeight = height;
    ImGui::SetNextWindowPos(ImVec2(GetApplication().GetWindowWidth() / 2 - winWidth / 2,
               3 * GetApplication().GetWindowHeight() / 3 - winHeight * 1.5f));

    ImGui::Begin("Conquer Space", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);

    float buttonHeight = 24;
    float buttonWidth = 600 / 3 - 5;  // Subtract some space for padding
    float buttonWidthSubtraction = 5;

    ImGui::BeginTable("table1", 6, ImGuiTableFlags_NoPadOuterX);
    ImGui::TableNextColumn();
    if (ImGui::Button("New Game", ImVec2(buttonWidth, buttonHeight))) {
        // Switch scene to new game menu
        m_new_game_window = true;
    }
    ImGui::TableNextColumn();

    int grey = 75;
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(grey, grey, grey)));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(grey, grey, grey)));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(grey, grey, grey)));
    if (ImGui::Button("Load Save", ImVec2(buttonWidth, buttonHeight)), ImGuiButtonFlags_None) {
        // Get save game
        m_save_game_window = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::TableNextColumn();
    ImGui::Button("Options", ImVec2(buttonWidth, buttonHeight));
    ImGui::TableNextColumn();

    if (ImGui::Button("Credits", ImVec2(buttonWidth, buttonHeight))) {
        // Then show credits window
        m_credits_window = true;
    }

    ImGui::TableNextColumn();
    ImGui::Button("Others", ImVec2(buttonWidth, buttonHeight));
    ImGui::TableNextColumn();
    if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight))) {
        GetApplication().ExitApplication();
    }
    ImGui::EndTable();

    width = ImGui::GetWindowWidth();
    height = ImGui::GetWindowHeight();
    ImGui::End();

    if (m_new_game_window) {
        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("New Game", &m_new_game_window,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("New Game")) {
            // Switch scene
            GetApplication().SetScene<conquerspace::scene::UniverseLoadingScene>();
        }
        ImGui::End();
    }

    if (m_save_game_window) {
        ImGui::Begin("Load Game", &m_credits_window, ImGuiWindowFlags_NoCollapse);
        ImGui::End();
    }

    if (m_credits_window) {
        ImGui::SetNextWindowSize(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.8f, ImGui::GetIO().DisplaySize.y * 0.8f),
            ImGuiCond_Always);
        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Credits", &m_credits_window, ImGuiWindowFlags_NoCollapse);
        ImGui::Markdown(m_credits->data.c_str(), m_credits->data.length(),
            GetApplication().markdownConfig);
        ImGui::End();
    }
}

void conquerspace::scene::MainMenuScene::Render(float deltaTime) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    splashscreen.shaderProgram->UseProgram();
    glm::mat4 mat = glm::mat4(1.f);
    mat = glm::scale(mat, glm::vec3(1, ratio, 1));
    glm::translate(mat, glm::vec3(0.05, 0.05, 1));
    splashscreen.shaderProgram->setMat4("transform", mat);
    conquerspace::engine::Draw(splashscreen);

    mat = glm::mat4(1.f);
    mat = glm::translate(mat, glm::vec3(-0.6, 0.75, 1));
    mat = glm::scale(mat, glm::vec3(1, ratio, 1));
    mat = glm::scale(mat, glm::vec3(1, ratio2, 1));
    mat = glm::scale(mat, glm::vec3(0.35, 0.35, 1));
    titleBanner.shaderProgram->setMat4("transform", mat);
    conquerspace::engine::Draw(titleBanner);
}
