/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <memory>

#include <boost/atomic.hpp>
#include <boost/thread.hpp>

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

    boost::atomic<bool> m_done_loading;

    std::unique_ptr<boost::thread> thread;

    boost::atomic<float> percentage;

    conquerspace::asset::AssetLoader assetLoader;
};
}  // namespace scene
}  // namespace conquerspace
