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
#include "client/scenes/fieldviewerscene.h"

#include "client/systems/assetloading.h"
#include "client/systems/sysfieldviewer.h"

cqsp::scene::ObjectEditorScene::ObjectEditorScene(cqsp::engine::Application& app) : cqsp::engine::Scene(app) {
    AddUISystem<cqsp::client::systems::SysFieldNodeViewer>();
}

cqsp::scene::ObjectEditorScene::~ObjectEditorScene() {}

void cqsp::scene::ObjectEditorScene::Init() {
    // Sorta need to initialize everything
    cqsp::client::systems::LoadAllResources(GetApp());
}

void cqsp::scene::ObjectEditorScene::Update(float deltaTime) {}

void cqsp::scene::ObjectEditorScene::Ui(float deltaTime) {
    for (auto& ui : user_interfaces) {
        ui->DoUI(deltaTime);
    }
}

void cqsp::scene::ObjectEditorScene::Render(float deltaTime) {}
