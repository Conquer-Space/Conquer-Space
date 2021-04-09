/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "engine/application.h"
#include "engine/engine.h"

namespace conquerspace {
namespace engine {
class Scene {
 public:
    explicit Scene(Application& app);

    Application& GetApplication() { return m_application; }

    virtual void Init() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Ui(float deltaTime) = 0;
    virtual void Render(float deltaTime) = 0;

 private:
    Application& m_application;
};

class EmptyScene : public Scene {
 public:
    explicit EmptyScene(Application& app);

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);
};
}  // namespace engine
}  // namespace conquerspace
