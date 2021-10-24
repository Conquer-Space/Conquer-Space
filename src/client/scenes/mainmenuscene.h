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
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/renderer2d.h"
#include "engine/renderer/primitives/pane.h"
#include "engine/renderer/text.h"

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

    cqsp::asset::TextAsset* m_credits;

    bool m_credits_window = false;
    bool m_save_game_window = false;
    bool m_new_game_window = false;
    bool m_options_window = false;
    bool m_show_mods_window = false;

    float ratio, ratio2;
    float width, height;

    cqsp::engine::Renderer2D* object_renderer;
    cqsp::engine::Mesh rectangle;

    cqsp::asset::Texture* splash_screen;
    cqsp::asset::Texture* title_banner_texture;
    cqsp::asset::ShaderProgram_t program;

};
}  // namespace scene
}  // namespace cqsp
