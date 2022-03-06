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

#include "engine/scene.h"
#include "engine/graphics/renderable.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/renderer2d.h"
#include "engine/graphics/primitives/pane.h"
#include "engine/graphics/text.h"

#include "client/systems/settingswindow.h"
#include "client/systems/creditswindow.h"

namespace cqsp {
namespace scene {
class MainMenuScene : public cqsp::engine::Scene{
 public:
    explicit MainMenuScene(cqsp::engine::Application& app);
    ~MainMenuScene();

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    void ModWindow();

    Rml::ElementDocument* main_menu;

    client::SettingsWindow settings_window;
    client::CreditsWindow credits_window;

    class EventListener : public Rml::EventListener {
     public:
        EventListener() = default;
        void ProcessEvent(Rml::Event& event);
        cqsp::engine::Application* app;
        MainMenuScene* m_scene;
    } listener;

    bool is_options_visible = false;
    bool last_options_visible = false;
};
}  // namespace scene
}  // namespace cqsp
