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
#include <utility>
#include <memory>

#include "engine/renderer/renderer.h"
#include "engine/window.h"

namespace cqsp {
namespace engine {

// Warning: does not work, please don't use this
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

class Framebuffer {
 public:
    virtual void InitTexture(int width = 1280, int height = 720) = 0;
    virtual void Clear() = 0;
    virtual void BeginDraw() = 0;
    virtual void EndDraw() = 0;
    virtual void RenderBuffer() = 0;
    virtual void Free() = 0;
    virtual void NewFrame(const Window& window) = 0;

    virtual cqsp::engine::Mesh& GetMeshOutput() = 0;
    virtual void SetShader(cqsp::asset::ShaderProgram_t shader) = 0;
};

class FramebufferRenderer : public Framebuffer {
 public:
    FramebufferRenderer() : Framebuffer() {}

    void InitTexture(int width  = 1280, int height = 720) override;
    void Clear() override;
    void BeginDraw() override;
    void EndDraw() override;
    void RenderBuffer() override;
    void Free() override;
    void NewFrame(const Window& window) override;
    cqsp::engine::Mesh& GetMeshOutput() override { return mesh_output; }
    void SetShader(cqsp::asset::ShaderProgram_t shader) override {
        buffer_shader = shader;
    }

 private:
    unsigned int framebuffer;
    unsigned int colorbuffer;
    cqsp::asset::ShaderProgram_t buffer_shader;
    cqsp::engine::Mesh mesh_output;
};

class AAFrameBufferRenderer : public Framebuffer {
 public:
    AAFrameBufferRenderer() : Framebuffer() {}

    void InitTexture(int width = 1280, int height = 720);
    void Clear() override;
    void BeginDraw() override;
    void EndDraw() override;
    void Free() override;
    void RenderBuffer() override;
    void NewFrame(const Window& window) override;

    cqsp::engine::Mesh& GetMeshOutput() override { return mesh_output; }

    void SetShader(cqsp::asset::ShaderProgram_t shader) override {
        buffer_shader = shader;
    }

 private:
    int width;
    int height;

    unsigned int framebuffer;
    unsigned int intermediateFBO;
    unsigned int screenTexture;
    unsigned int textureColorBufferMultiSampled;
    cqsp::asset::ShaderProgram_t buffer_shader;
    cqsp::engine::Mesh mesh_output;
};

class LayerRenderer {
 public:
    template<class T>
    int AddLayer(asset::ShaderProgram_t shader, const engine::Window& window) {
        std::unique_ptr<T> fb = std::make_unique<T>();
        InitFramebuffer(fb.get(), shader, window);
        framebuffers.push_back(std::move(fb));
        return framebuffers.size() - 1;
    }

    void BeginDraw(int layer);
    void EndDraw(int layer);
    void DrawAllLayers();
    void NewFrame(const cqsp::engine::Window& window);
    int GetLayerCount();

 private:
    std::vector<std::unique_ptr<Framebuffer>> framebuffers;
    void InitFramebuffer(Framebuffer* buffer, cqsp::asset::ShaderProgram_t shader,
                            const cqsp::engine::Window& window);
};
}  // namespace engine
}  // namespace cqsp
