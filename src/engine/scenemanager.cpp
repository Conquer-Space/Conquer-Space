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
#include "engine/scenemanager.h"

#include <memory>
#include <utility>

#include <tracy/Tracy.hpp>

#include "engine/enginelogger.h"
#include "engine/scene.h"

namespace cqsp::engine {

void SceneManager::SetInitialScene(std::unique_ptr<Scene> scene) { m_scene = std::move(scene); }

void SceneManager::SetScene(std::unique_ptr<Scene> scene) {
    m_next_scene = std::move(scene);
    m_switch = true;
}

void SceneManager::SwitchScene() {
    m_scene = std::move(m_next_scene);
    ENGINE_LOG_TRACE("Initializing scene");
    m_scene->Init();
    ENGINE_LOG_TRACE("Done Initializing scene");
    m_switch = false;
}

Scene* SceneManager::GetScene() { return m_scene.get(); }

void SceneManager::DeleteCurrentScene() { m_scene.reset(); }

void SceneManager::Update(float deltaTime) {
    ZoneScoped;
    m_scene->Update(deltaTime);
}

void SceneManager::Ui(float deltaTime) {
    ZoneScoped;
    m_scene->Ui(deltaTime);
}

void SceneManager::Render(float deltaTime) {
    ZoneScoped;
    m_scene->Render(deltaTime);
}
}  // namespace cqsp::engine
