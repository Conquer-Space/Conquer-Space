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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iomanip>

#include "engine/application.h"
#include "engine/engine.h"
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

    std::unique_ptr<MockScene> initial = std::make_unique<MockScene>();
    std::unique_ptr<MockScene> new_scene = std::make_unique<MockScene>();

    // Initial scene is not initialized, because it will be an empty scene.
    EXPECT_CALL(*new_scene, Init()).Times(1);

    // Before scene
    EXPECT_CALL(*initial, Update(0)).Times(1);
    EXPECT_CALL(*new_scene, Update(0)).Times(0);

    // Switch scene
    EXPECT_CALL(*initial, Update(1)).Times(0);
    EXPECT_CALL(*new_scene, Update(1)).Times(1);

    scene_manager.SetInitialScene(std::move(initial));
    EXPECT_FALSE(scene_manager.ToSwitchScene());

    scene_manager.SetScene(std::move(new_scene));
    EXPECT_TRUE(scene_manager.ToSwitchScene());

    // Ensure that scene is not switched yet so that the game doesn't switch scenes
    // in the middle of a frame
    scene_manager.Update(0);

    scene_manager.SwitchScene();
    EXPECT_FALSE(scene_manager.ToSwitchScene());

    scene_manager.Update(1);
}
