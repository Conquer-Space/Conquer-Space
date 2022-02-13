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
#include "client/scenes/texttestscene.h"

#include <glad/glad.h>

#define TX(x) \
    indices[#x] = x; \
    names.push_back(#x);
cqsp::scene::TextTestScene::~TextTestScene() {}

void cqsp::scene::TextTestScene::Init() {
    auto shader = GetApp().GetAssetManager().MakeShader("core:framebuffervert", "core:framebufferfrag");
    layer = renderer.AddLayer<cqsp::engine::AAFrameBufferRenderer>(shader, *GetApp().GetWindow());

    TX(GL_ONE);
    TX(GL_SRC_COLOR);
    TX(GL_ONE_MINUS_SRC_COLOR);
    TX(GL_DST_COLOR);
    TX(GL_ONE_MINUS_DST_COLOR);
    TX(GL_SRC_ALPHA);
    TX(GL_ONE_MINUS_SRC_ALPHA);
    TX(GL_DST_ALPHA);
    TX(GL_ONE_MINUS_DST_ALPHA);
    TX(GL_CONSTANT_COLOR);
    TX(GL_ONE_MINUS_CONSTANT_COLOR);
    TX(GL_CONSTANT_ALPHA);
    TX(GL_ONE_MINUS_CONSTANT_ALPHA);
    TX(GL_SRC_ALPHA_SATURATE);
    TX(GL_SRC1_COLOR);
    TX(GL_ONE_MINUS_SRC1_COLOR);
    TX(GL_SRC1_ALPHA);
    TX(GL_ONE_MINUS_SRC1_ALPHA);
}

void cqsp::scene::TextTestScene::Update(float deltaTime) {
    //delta_t += deltaTime;
    if (delta_t > 0.1f) {
        src++;
        if (src >= names.size()) {
            src = 0;
            post++;
        }
        if (post >= names.size()) {
            src = 0;
            post = 0;
        }
        delta_t = 0;
    }
}

void cqsp::scene::TextTestScene::Ui(float deltaTime) {
    ImGui::Begin("Text config");
    ImGui::TextFmt("{}", delta_t);
    ImGui::DragFloat("Font size", &font_size, 0, 255);
    if (ImGui::BeginCombo("combo 1", names[src])) {
            for (int n = 0; n < names.size(); n++) {
                bool is_selected = (src == n);
                if (ImGui::Selectable(names[n], is_selected)) {
                    src = n;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("combo 2", names[post])) {
            for (int n = 0; n < names.size(); n++) {
                bool is_selected = (post == n);
                if (ImGui::Selectable(names[n], is_selected)) {
                    post = n;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    ImGui::End();
}

void cqsp::scene::TextTestScene::Render(float deltaTime) {
    // Set a cool background color
    renderer.NewFrame(*GetApp().GetWindow());
    glClearColor(1.f, 0.f, 1.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    renderer.BeginDraw(layer);
    GetApp().DrawText(text, 20, GetApp().GetWindowHeight()/2, font_size);
    renderer.DrawAllLayers();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GetApp().DrawText(fmt::format("2: {}", text), 20, GetApp().GetWindowHeight() / 2 - 50, font_size);
}
