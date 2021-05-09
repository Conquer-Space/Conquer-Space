/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "client/systems/sysstarsystemrenderer.h"
#include "engine/scene.h"
#include "engine/application.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"

namespace conquerspace {
namespace scene {
class UniverseScene : public conquerspace::engine::Scene {
 public:
    explicit UniverseScene(conquerspace::engine::Application& app);
    ~UniverseScene() {
        delete system_renderer;
    }

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    conquerspace::engine::Renderable sphere;
    conquerspace::engine::Renderable sky;
    conquerspace::engine::Renderable planetDisp;
    conquerspace::engine::Renderable sun;

    float x = 0, y = 0;

    double previous_mouseX;
    double previous_mouseY;

    entt::entity player;

    conquerspace::client::SysStarSystemRenderer* system_renderer;
};
}  // namespace scene
}  // namespace conquerspace
