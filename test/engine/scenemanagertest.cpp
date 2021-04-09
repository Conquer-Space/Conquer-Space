/*
 * Copyright 2021 Conquer Space
 */
#include <gtest/gtest.h>

#include <memory>

#include "engine/engine.h"
#include "engine/application.h"
#include "engine/scene.h"

namespace cqspe = conquerspace::engine;

class TestScene1 : public cqspe::Scene {
 public:
    explicit TestScene1(cqspe::Application& app) : Scene(app) {}

    void Init() {}
    void Update(float deltaTime) { value++; }
    void Ui(float deltaTime) {}
    void Render(float deltaTime) {}

    int value = 10;
};

class TestScene2 : public cqspe::Scene {
 public:
    explicit TestScene2(cqspe::Application& app) : Scene(app) {}
    void Init() {}
    void Update(float deltaTime) { value++; }
    void Ui(float deltaTime) {}
    void Render(float deltaTime) {}

    int value = 1;
};

TEST(SceneManagerTest, changeSceneTest) {
    cqspe::Application test_application;
    cqspe::SceneManager scene_manager;

    std::shared_ptr<TestScene1> initial =
        std::make_shared<TestScene1>(test_application);
    scene_manager.SetInitialScene(std::dynamic_pointer_cast<cqspe::Scene>(initial));
    scene_manager.GetScene()->Update(0.f);

    ASSERT_EQ(11, initial->value);
    ASSERT_EQ(false, scene_manager.ToSwitchScene());

    std::shared_ptr<TestScene2> new_scene =
        std::make_shared<TestScene2>(test_application);
    scene_manager.SetScene(std::dynamic_pointer_cast<cqspe::Scene>(new_scene));

    ASSERT_EQ(true, scene_manager.ToSwitchScene());

    scene_manager.SwitchScene();

    ASSERT_EQ(false, scene_manager.ToSwitchScene());

    scene_manager.GetScene()->Update(0.f);
    ASSERT_EQ(11, initial->value);
    ASSERT_EQ(2, new_scene->value);

    scene_manager.GetScene()->Update(0.f);
    ASSERT_EQ(11, initial->value);
    ASSERT_EQ(3, new_scene->value);
}
