/*
 * Copyright 2021 Conquer Space
 */
#include "engine/scene.h"

#include <imgui.h>

conquerspace::engine::Scene::Scene(Application& app)
    : m_application(app) {}

conquerspace::engine::EmptyScene::EmptyScene(Application& app)
    : Scene(app) {}

void conquerspace::engine ::EmptyScene::Init() {}

void conquerspace::engine::EmptyScene::Update(float deltaTime) {}

void conquerspace::engine::EmptyScene::Ui(float deltaTime) {}

void conquerspace::engine::EmptyScene::Render(float deltaTime) {}
