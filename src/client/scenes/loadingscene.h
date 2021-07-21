/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <memory>

#include <atomic>
#include <thread>

#include "engine/scene.h"

namespace conquerspace {
namespace scene {
class LoadingScene : public conquerspace::engine::Scene {
 public:
    explicit LoadingScene(conquerspace::engine::Application& app);
    ~LoadingScene() {}

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

    conquerspace::asset::AssetLoader assetLoader;

    void LoadFont();
};
}  // namespace scene
}  // namespace conquerspace
