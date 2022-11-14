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
#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "engine/scene.h"

namespace cqsp {
namespace scene {
class UniverseLoadingScene : public cqsp::engine::Scene {
 public:
    explicit UniverseLoadingScene(cqsp::engine::Application& app);
    ~UniverseLoadingScene();

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    std::atomic<bool> m_done_loading;
    std::unique_ptr<std::thread> thread;

    void LoadUniverse();

    bool m_completed_loading;

    Rml::ElementDocument* document;
};
}  // namespace scene
}  // namespace cqsp
