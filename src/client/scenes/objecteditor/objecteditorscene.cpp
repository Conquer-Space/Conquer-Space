/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "objecteditorscene.h"

#include "client/systems/assetloading.h"
#include "goodviewer.h"
#include "recipeviewer.h"
#include "sysfieldviewer.h"

using cqsp::scene::ObjectEditorScene;
using entt::entity;

ObjectEditorScene::ObjectEditorScene(engine::Application& app) : client::Scene(app) {
    AddUISystem<client::systems::SysFieldNodeViewer>("Node Viewer");
    AddUISystem<client::systems::SysGoodViewer>("Good Viewer");
    AddUISystem<client::systems::SysRecipeViewer>("Recipe Viewer");
}

ObjectEditorScene::~ObjectEditorScene() = default;

void ObjectEditorScene::Init() {
    // Sorta need to initialize everything
    client::systems::LoadAllResources(GetApp(), *dynamic_cast<client::ConquerSpace*>(GetApp().GetGame()));
}

void ObjectEditorScene::Update(float deltaTime) {}

void ObjectEditorScene::Ui(float deltaTime) {
    // Do information UI
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::Begin("UI to Show");
    if (ImGui::Button("Exit")) {
        // Exit app
        GetApp().ExitApplication();
    }
    for (auto& ui : user_interfaces) {
        ImGui::Checkbox(fmt::format("{}", ui.first).c_str(), &ui.second.first);
    }
    ImGui::End();
    for (auto& ui : user_interfaces) {
        if (ui.second.first) {
            ui.second.second->DoUI(deltaTime);
        }
    }
}

void ObjectEditorScene::Render(float deltaTime) {}
