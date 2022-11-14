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
#include "engine/scene.h"

#include "engine/gui.h"

cqsp::engine::Scene::Scene(Application& app) : m_application(&app) {}

cqsp::common::Universe& cqsp::engine::Scene::GetUniverse() { return GetApp().GetUniverse(); }

cqsp::asset::AssetManager& cqsp::engine::Scene::GetAssetManager() { return GetApp().GetAssetManager(); }

cqsp::engine::EmptyScene::EmptyScene(Application& app) : Scene(app) {}

void cqsp::engine ::EmptyScene::Init() {}

void cqsp::engine::EmptyScene::Update(float deltaTime) {}

void cqsp::engine::EmptyScene::Ui(float deltaTime) {}

void cqsp::engine::EmptyScene::Render(float deltaTime) {}
