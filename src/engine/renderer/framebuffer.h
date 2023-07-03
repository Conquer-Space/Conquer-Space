/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include <utility>
#include <vector>

#include "engine/renderer/renderer.h"
#include "engine/window.h"

namespace cqsp {
namespace engine {
/// <summary>
/// Interface for framebuffer renderers.
/// </summary>
class IFramebuffer {
 public:
    virtual ~IFramebuffer() {}

    virtual void InitTexture(int width = 1280, int height = 720) = 0;
    virtual void Clear() = 0;
    virtual void BeginDraw() = 0;
    virtual void EndDraw() = 0;
    virtual void RenderBuffer() = 0;
    virtual void Free() = 0;
    virtual void NewFrame(const Window& window) = 0;

    virtual cqsp::engine::Mesh& GetMeshOutput() = 0;
    virtual void SetMesh(cqsp::engine::Mesh*) = 0;
    virtual void SetShader(cqsp::asset::ShaderProgram_t shader) = 0;
};

class FramebufferRenderer : public IFramebuffer {
 public:
    FramebufferRenderer() : IFramebuffer() {}
    ~FramebufferRenderer();

    void InitTexture(int width = 1280, int height = 720) override;
    void Clear() override;
    void BeginDraw() override;
    void EndDraw() override;
    void RenderBuffer() override;
    void Free() override;
    void NewFrame(const Window& window) override;
    void SetMesh(cqsp::engine::Mesh* mesh) override { mesh_output = mesh; }
    cqsp::engine::Mesh& GetMeshOutput() override { return *mesh_output; }
    void SetShader(cqsp::asset::ShaderProgram_t shader) override { buffer_shader = shader; }

 private:
    unsigned int framebuffer;
    unsigned int colorbuffer;
    cqsp::asset::ShaderProgram_t buffer_shader;
    cqsp::engine::Mesh* mesh_output;
};

class AAFrameBufferRenderer : public IFramebuffer {
 public:
    AAFrameBufferRenderer() : IFramebuffer() {}
    ~AAFrameBufferRenderer();

    void InitTexture(int width = 1280, int height = 720) override;
    void Clear() override;
    void BeginDraw() override;
    void EndDraw() override;
    void Free() override;
    void RenderBuffer() override;
    void NewFrame(const Window& window) override;

    cqsp::engine::Mesh& GetMeshOutput() override { return *mesh_output; }
    void SetMesh(cqsp::engine::Mesh* mesh) override { mesh_output = mesh; }
    void SetShader(cqsp::asset::ShaderProgram_t shader) override { buffer_shader = shader; }

 private:
    int width;
    int height;

    unsigned int framebuffer;
    unsigned int intermediateFBO;
    unsigned int screenTexture;
    unsigned int mscat;
    cqsp::asset::ShaderProgram_t buffer_shader;
    cqsp::engine::Mesh* mesh_output;
};

/// <summary>
/// Renders a series of framebuffers onto screen. This is a relatively simple way of
/// allowing organization of multiple framebuffers. But layer management is a bit scuffed
/// and could be made more intuitive. How, that's a different question.
/// <br>
/// How to use:
/// <br>
/// ```
/// ShaderProgram_t shader; // Initialized somewhere above
/// // Init layer renderer
/// LayerRenderer layers;
/// // Add a new layer, and you can note down the index number
/// // The layers will be rendered in order of what you define it, so to reorganize layers
/// // you can just reorder the order they are initialized.
/// int first = layers.AddLayer<FramebufferRenderer>(shader, window);
/// int second = layers.AddLayer<FramebufferRenderer>(shader, window);
///
/// // Initialize the frame by doing this:
/// // .. Inside render loop
/// NewFrame(window); // Will clear all framebuffers
/// BeginDraw(first);
/// // .. some drawing
/// EndDraw(first);
/// BeginDraw(second);
/// // .. More drawing
/// EndDraw(second);
///
/// // Now this will be drawn at the end
/// DrawAllLayers();
/// ```
/// </summary>
class LayerRenderer {
 public:
    template <class T>
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

    IFramebuffer* GetFrameBuffer(int layer) { return framebuffers[layer].get(); }

 private:
    std::vector<std::unique_ptr<IFramebuffer>> framebuffers;
    void InitFramebuffer(IFramebuffer* buffer, cqsp::asset::ShaderProgram_t shader, const cqsp::engine::Window& window);
};
}  // namespace engine
}  // namespace cqsp
