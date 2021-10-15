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

#include <memory>

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

TEST(SceneManagerTest, changeSceneTest) {
    // Empty application that does nothing
    cqspe::Application test_application;
    cqspe::SceneManager scene_manager;

    std::shared_ptr<TestScene1> initial = std::make_shared<TestScene1>(test_application);
    scene_manager.SetInitialScene(std::static_pointer_cast<cqspe::Scene>(initial));
    scene_manager.GetScene()->Update(0.f);

    ASSERT_EQ(11, initial->value);
    ASSERT_EQ(false, scene_manager.ToSwitchScene());

    std::shared_ptr<TestScene2> new_scene = std::make_shared<TestScene2>(test_application);
    scene_manager.SetScene(std::static_pointer_cast<cqspe::Scene>(new_scene));

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
