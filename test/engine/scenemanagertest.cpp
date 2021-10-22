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

class MockScene : public cqspe::Scene {
 public:
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Update, (float deltaTime), (override));
    MOCK_METHOD(void, Ui, (float deltaTime), (override));
    MOCK_METHOD(void, Render, (float deltaTime), (override));
};

using ::testing::_;

TEST(SceneManagerTest, SceneChangeTest) {
    // Empty application that does nothing
    cqspe::SceneManager scene_manager;

    std::shared_ptr<MockScene> initial_ptr = std::make_shared<MockScene>();
    std::shared_ptr<MockScene> new_scene_ptr =  std::make_shared<MockScene>();

    scene_manager.SetInitialScene(std::static_pointer_cast<cqspe::Scene>(initial_ptr));
    EXPECT_FALSE(scene_manager.ToSwitchScene());

    scene_manager.SetScene(std::static_pointer_cast<cqspe::Scene>(new_scene_ptr));
    EXPECT_TRUE(scene_manager.ToSwitchScene());

    EXPECT_CALL(*new_scene_ptr, Update(_)).Times(0);
    EXPECT_CALL(*initial_ptr, Update(_)).Times(1);

    scene_manager.Update(0);

    EXPECT_CALL(*new_scene_ptr, Init()).Times(1);
    scene_manager.SwitchScene();

    EXPECT_CALL(*new_scene_ptr, Update(_)).Times(1);
    EXPECT_CALL(*initial_ptr, Update(_)).Times(0);
    scene_manager.Update(0);
}
