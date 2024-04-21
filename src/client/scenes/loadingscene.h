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
#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "client/scenes/scene.h"

namespace cqsp::scene {
// First loading scene when the game starts
class LoadingScene : public client::Scene {
 public:
    explicit LoadingScene(engine::Application& app);
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

    asset::AssetLoader assetLoader;

    Rml::ElementDocument* document;

    void LoadFont();
    bool need_halt = false;

    struct LoadingDataModel {
        int current = 0;
        int max = 0;
    } loading_data;

    Rml::DataModelHandle model_handle;
};
}  // namespace cqsp::scene
