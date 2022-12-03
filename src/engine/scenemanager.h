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
#pragma once

#include <memory>

namespace cqsp {
namespace engine {
class Scene;

/// <summary>
/// Manages scenes for the application.
/// </summary>
class SceneManager {
 public:
    /// <summary>
    /// Sets the current scene.
    /// </summary>
    /// <param name="scene"></param>
    void SetInitialScene(std::unique_ptr<Scene> scene);

    /// <summary>
    /// Sets the next scene, and the scene will be switched when <code>SwitchScene</code> is executed.
    /// </summary>
    void SetScene(std::unique_ptr<Scene> scene);

    /// <summary>
    /// Sets the next scene to the current.
    /// </summary>
    void SwitchScene();

    /// <summary>
    /// Gets current running scene.
    /// </summary>
    /// <returns></returns>
    Scene* GetScene();

    /// <summary>
    /// Verifies if it is appropiate to switch scenes.
    /// </summary>
    /// <returns></returns>
    bool ToSwitchScene() { return (m_switch && m_next_scene != nullptr); }

    void Update(float deltaTime);

    void Ui(float deltaTime);

    void Render(float deltaTime);

    void DeleteCurrentScene();

 private:
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Scene> m_next_scene;

    /// <summary>
    /// If the next scene has been set.
    /// </summary>
    bool m_switch;
};
}  // namespace engine
}  // namespace cqsp
