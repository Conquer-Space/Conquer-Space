/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "engine/scene.h"
#include "engine/application.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"

namespace conquerspace {
namespace scene {
class UniverseScene : public conquerspace::engine::Scene {
 public:
    explicit UniverseScene(conquerspace::engine::Application& app);
    ~UniverseScene() {}

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    conquerspace::engine::Renderable sphere;
    conquerspace::engine::Renderable sky;
    float distance = 5;
    float theta = 0;

    float x = 0, y = 0, scroll = 10;

    double previous_mouseX;
    double previous_mouseY;

    double viewAngleX = 0;
    double viewAngleY = 0;

    conquerspace::engine::BasicRenderer basic_renderer;
};
}  // namespace scene
}  // namespace conquerspace
