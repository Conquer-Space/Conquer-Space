/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "engine/engine.h"
#include "engine/application.h"
#include "engine/scene.h"

namespace cqspe = cqsp::engine;

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

class MockScene : public cqspe::Scene {
 public:
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Update, (float deltaTime), (override));
    MOCK_METHOD(void, Ui, (float deltaTime), (override));
    MOCK_METHOD(void, Render, (float deltaTime), (override));
};

TEST(SceneManagerTest, changeSceneTest) {
    // Empty application that does nothing
    cqspe::SceneManager scene_manager;

    MockScene initial;
    std::shared_ptr<MockScene> initial_ptr(&initial);
    scene_manager.SetInitialScene(std::static_pointer_cast<cqspe::Scene>(initial_ptr));

    MockScene new_scene;
    std::shared_ptr<MockScene> new_scene_ptr(&new_scene);

    //scene_manager.SetScene(std::static_pointer_cast<cqspe::Scene>(new_scene_ptr));
    EXPECT_CALL(new_scene, Init);
    scene_manager.SwitchScene();
    // Clean up
    scene_manager.GetScene().reset();
}
