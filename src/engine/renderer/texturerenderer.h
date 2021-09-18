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

#include <vector>

#include "engine/renderer/renderer.h"
#include "engine/window.h"

namespace cqsp {
namespace engine {
class TextureRenderer {
 public:
    glm::mat4 projection = glm::mat4(1.0);
    glm::mat4 view = glm::mat4(1.0);
    std::vector<BasicRendererObject> renderables;
    cqsp::asset::ShaderProgram* buffer_shader;
    cqsp::engine::Mesh* mesh_output;
    unsigned int framebuffer = 0;
    void Draw();
    void RenderBuffer();
};

class FramebufferRenderer {
 public:
    unsigned int framebuffer;
    unsigned int colorbuffer;
    cqsp::asset::ShaderProgram buffer_shader;
    cqsp::engine::Mesh mesh_output;
    void InitTexture(int width  = 1280, int height = 720);
    void Clear();
    void BeginDraw();
    void EndDraw();
    void RenderBuffer();
    void Free();
    void NewFrame(Window& window);
};

/*
* In progress, not working.
*/
class AAFrameBufferRenderer {
 public:
    unsigned int framebuffer;
    unsigned int intermediateFBO;
    unsigned int screenTexture;
    unsigned int textureColorBufferMultiSampled;
    cqsp::asset::ShaderProgram buffer_shader;
    cqsp::engine::Mesh mesh_output;
    void InitTexture(int width  = 1280, int height = 720);
    void Clear();
    void BeginDraw();
    void EndDraw();
    void Free();
    void RenderBuffer();
    void NewFrame(Window& window);

    int width;
    int height;
};
}  // namespace engine
}  // namespace cqsp
