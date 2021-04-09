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
class UniverseLoadingScene : public conquerspace::engine::Scene {
 public:
    explicit UniverseLoadingScene(conquerspace::engine::Application& app);
    ~UniverseLoadingScene() {}

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    boost::atomic<bool> m_done_loading;
    std::unique_ptr<boost::thread> thread;

    void LoadUniverse();
};
}  // namespace scene
}  // namespace conquerspace
