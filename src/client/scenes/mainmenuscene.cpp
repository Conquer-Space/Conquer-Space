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
#include "client/scenes/mainmenuscene.h"

#include <glad/glad.h>

#include <imgui_markdown.h>

#include <utility>
#include <algorithm>
#include <filesystem>

#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/shader.h"
#include "engine/renderer/renderer.h"
#include "client/scenes/universeloadingscene.h"
#include "engine/asset.h"
#include "engine/gui.h"
#include "engine/cqspgui.h"
#include "engine/renderer/text.h"
#include "client/systems/sysoptionswindow.h"
#include "common/version.h"

#include "common/util/paths.h"

cqsp::scene::MainMenuScene::MainMenuScene(cqsp::engine::Application& app)
    : cqsp::engine::Scene(app) { }

cqsp::scene::MainMenuScene::~MainMenuScene() {
    SPDLOG_INFO("Deleting main menu");
}

void cqsp::scene::MainMenuScene::Init() {
    using cqsp::asset::Texture;
    m_texture = GetAssetManager().GetAsset<cqsp::asset::Texture>("core:cqspbanner");

    m_credits = GetAssetManager().GetAsset<cqsp::asset::TextAsset>("core:credits");

    splashscreen = cqsp::engine::MakeRenderable();
    titleBanner = cqsp::engine::MakeRenderable();

    cqsp::primitive::MakeTexturedPaneMesh(*splashscreen);
    cqsp::primitive::MakeTexturedPaneMesh(*titleBanner);

    cqsp::asset::Texture* earthrise_texture = GetAssetManager().GetAsset<Texture>("core:earthrise");

    cqsp::asset::Texture* asset2 = GetAssetManager().GetAsset<Texture>("core:title");

    // Create new shader program
    cqsp::asset::ShaderProgram_t program = GetAssetManager().
        MakeShader("core:shader.pane.vert", "core:shader.texturedpane.frag");

    splashscreen->shaderProgram = program;
    titleBanner->shaderProgram = program;

    // Make shaders
    splashscreen->SetTexture("texture1", 0, earthrise_texture);
    titleBanner->SetTexture("texture1", 0, asset2);

    glm::mat4 mat = glm::mat4(1.f);
    glm::translate(mat, glm::vec3(0.05, 0.05, 1));
    splashscreen->model = mat;
    renderer.renderables.push_back(std::move(splashscreen));

    mat = glm::mat4(1.f);
    mat = glm::translate(mat, glm::vec3(-0.6, 0.45, 1));
    // Resize for rectangle
    mat = glm::scale(mat,
                    glm::vec3(1, static_cast<float>(asset2->height)/
                            static_cast<float>(asset2->width), 1));
    mat = glm::scale(mat, glm::vec3(0.35, 0.35, 1));
    titleBanner->model = mat;
    renderer.renderables.push_back(std::move(titleBanner));

    mat = glm::mat4(1.f);
    mat = glm::scale(mat,
            glm::vec3(1, static_cast<float>(GetApp().GetWindowWidth())
                                /static_cast<float>(GetApp().GetWindowHeight()), 1));
    renderer.projection = mat;
}

void cqsp::scene::MainMenuScene::Update(float deltaTime) {}

void cqsp::scene::MainMenuScene::Ui(float deltaTime) {
    float winWidth = width;
    float winHeight = height;
    ImGui::SetNextWindowPos(ImVec2(GetApp().GetWindowWidth() / 2 - winWidth / 2,
               3 * GetApp().GetWindowHeight() / 3 - winHeight * 1.5f));

    ImGui::Begin("Conquer Space", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);

    float buttonHeight = 24;
    float buttonWidth = GetApp().GetWindowWidth()/6 - 5 * 6;  // Subtract some space for padding

    ImGui::BeginTable("table1", 6, ImGuiTableFlags_NoPadOuterX);
    ImGui::TableNextColumn();
    if (CQSPGui::DefaultButton("New Game", ImVec2(buttonWidth, buttonHeight))) {
        // Switch scene to new game menu
        m_new_game_window = true;
    }
    ImGui::TableNextColumn();

    int grey = 75;
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(grey, grey, grey)));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(grey, grey, grey)));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(grey, grey, grey)));
    if (CQSPGui::DefaultButton("Load Save", ImVec2(buttonWidth, buttonHeight)), ImGuiButtonFlags_None) {
        // Get save game
        m_save_game_window = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::TableNextColumn();
    if (CQSPGui::DefaultButton("Options", ImVec2(buttonWidth, buttonHeight))) {
        m_options_window = true;
    }
    ImGui::TableNextColumn();

    if (CQSPGui::DefaultButton("Credits", ImVec2(buttonWidth, buttonHeight))) {
        // Then show credits window
        m_credits_window = true;
    }

    ImGui::TableNextColumn();
    if (CQSPGui::DefaultButton("Mods", ImVec2(buttonWidth, buttonHeight))) {
        // Then load the mods
        m_show_mods_window = true;
    }

    ImGui::TableNextColumn();
    if (CQSPGui::DefaultButton("Quit", ImVec2(buttonWidth, buttonHeight))) {
        GetApp().ExitApplication();
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
        if (CQSPGui::DefaultButton("New Game")) {
            // Switch scene
            GetApp().SetScene<cqsp::scene::UniverseLoadingScene>();
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
            GetApp().markdownConfig);
        ImGui::End();
    }

    if (m_options_window) {
        cqsp::client::systems::ShowOptionsWindow(&m_options_window, GetApp());
    }

    if (m_show_mods_window) {
        ModWindow();
    }
}

void cqsp::scene::MainMenuScene::Render(float deltaTime) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer.Draw();
    GetApp().DrawText(fmt::format("Version: {}", CQSP_VERSION_STRING), 8, 8);
}

void cqsp::scene::MainMenuScene::ModWindow() {
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.8f, ImGui::GetIO().DisplaySize.y * 0.8f),
        ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Mods", &m_show_mods_window);
    auto& asset_manager = GetAssetManager();

    int height = ImGui::GetIO().DisplaySize.y * 0.8f - 75;
    ImGui::BeginChild("modlist", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f -
                                ImGui::GetStyle().ItemSpacing.y, height));
    static cqsp::asset::PackagePrototype* package = nullptr;
    bool selected = false;
    // Search bar
    static std::string search;
    ImGui::PushItemWidth(-1);
    ImGui::InputText("###mod search bar", &search);

    if (ImGui::BeginTable("modtable", 3, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("###[]", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 25.);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoReorder);
        ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_NoReorder);
        static bool enable_disable_all_mods = false;
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableSetColumnIndex(0);
        const char* column_name = ImGui::TableGetColumnName(0); // Retrieve name passed to TableSetupColumn()
        ImGui::PushID(0);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::Checkbox("##checkall", &enable_disable_all_mods)) {
            for (auto it = asset_manager.potential_mods.begin(); it != asset_manager.potential_mods.end(); it++) {
                it->second.enabled = enable_disable_all_mods;
            }
        }
        ImGui::PopStyleVar();
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TableHeader(column_name);
        ImGui::PopID();
        ImGui::TableSetColumnIndex(1);
        ImGui::PushID(1);
        ImGui::TableHeader("Name");
        ImGui::PopID();
        ImGui::TableSetColumnIndex(2);
        ImGui::PushID(1);
        ImGui::TableHeader("Info");
        ImGui::PopID();

        enable_disable_all_mods = true;
        for (auto it = asset_manager.potential_mods.begin(); it != asset_manager.potential_mods.end(); it++) {
            // Search to ignore case
            if (std::search(it->second.title.begin(), it->second.title.end(),
                    search.begin(), search.end(), [](char ch1, char ch2) {
                        return std::toupper(ch1) == std::toupper(ch2);
                    }) != it->second.title.end()) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                CQSPGui::DefaultCheckbox(fmt::format("###{}", it->second.name).c_str(), &it->second.enabled);
                ImGui::TableNextColumn();
                ImGui::Text(fmt::format("{}", it->second.title).c_str());
                ImGui::TableNextColumn();
                if (ImGui::SmallButton(fmt::format("Information##{}", it->second.title).c_str())) {
                    package = &it->second;
                }
                enable_disable_all_mods &= it->second.enabled;
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("PackageInfo", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f -
                                ImGui::GetStyle().ItemSpacing.y, height));
    if (package != nullptr) {
        ImGui::TextFmt("{}", package->title);
        ImGui::Separator();
        ImGui::TextFmt("Version: {}", package->version);
        ImGui::TextFmt("Author: {}", package->author);
    }
    ImGui::EndChild();

    if (ImGui::Button("Apply")) {
        asset_manager.SaveModList();
    }
    ImGui::SameLine();
    ImGui::End();
}
