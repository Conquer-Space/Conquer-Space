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

#include <memory>

#include <atomic>
#include <thread>

#include "engine/scene.h"

namespace cqsp {
namespace scene {
class LoadingScene : public cqsp::engine::Scene {
 public:
    explicit LoadingScene(cqsp::engine::Application& app);
   ~LoadingScene();

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

    /*
    * Function to load in a thread.
    */
    void LoadResources();

 private:
    float windowWidth, windowHeight;

    std::atomic<bool> m_done_loading;

    std::unique_ptr<std::thread> thread;

    std::atomic<float> percentage;

    cqsp::asset::AssetLoader assetLoader;

    Rml::ElementDocument* document;

    void LoadFont();
    bool need_halt = false;

    struct LoadingDataModel {
        int current = 0;
        int max = 0;
    } loading_data;

    Rml::DataModelHandle model_handle;
};
}  // namespace scene
}  // namespace cqsp
