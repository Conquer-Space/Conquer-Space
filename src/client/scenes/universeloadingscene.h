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
class UniverseLoadingScene : public conquerspace::engine::Scene {
 public:
    explicit UniverseLoadingScene(conquerspace::engine::Application& app);
    ~UniverseLoadingScene() {}

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    std::atomic<bool> m_done_loading;
    std::unique_ptr<std::thread> thread;

    void LoadUniverse();

    bool m_completed_loading;
};
}  // namespace scene
}  // namespace conquerspace
