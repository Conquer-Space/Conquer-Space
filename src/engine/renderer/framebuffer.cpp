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
#include "engine/renderer/framebuffer.h"

#include <glad/glad.h>

#include <spdlog/spdlog.h>

#include <Tracy.hpp>

#include "common/util/profiler.h"
#include "engine/graphics/primitives/pane.h"

void GenerateFrameBuffer(unsigned int &framebuffer) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

cqsp::engine::FramebufferRenderer::~FramebufferRenderer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteBuffers(1, &colorbuffer);
}

void cqsp::engine::FramebufferRenderer::InitTexture(int width, int height) {
    GenerateFrameBuffer(framebuffer);
    // create a color attachment texture
    glGenTextures(1, &colorbuffer);
    glBindTexture(GL_TEXTURE_2D, colorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // use a single renderbuffer object for both a depth AND stencil buffer.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    // now actually attach it
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SPDLOG_ERROR("Framebuffer is not complete!");
    }
    // Reset framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::FramebufferRenderer::Clear() {
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void cqsp::engine::FramebufferRenderer::BeginDraw() {
    ZoneScoped;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void cqsp::engine::FramebufferRenderer::EndDraw() {
    ZoneScoped;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::FramebufferRenderer::RenderBuffer() {
    buffer_shader->UseProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorbuffer);

    mesh_output->Draw();

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void cqsp::engine::FramebufferRenderer::Free() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteBuffers(1, &colorbuffer);
}

void cqsp::engine::FramebufferRenderer::NewFrame(const Window& window) {
    ZoneScoped;
    BeginDraw();
    Clear();
    EndDraw();
    // Check if window size changed, and then change the window size.
    if (window.WindowSizeChanged()) {
        ZoneScoped;
        // Then resize window
        Free();
        InitTexture(window.GetWindowWidth(), window.GetWindowHeight());
    }
}

cqsp::engine::AAFrameBufferRenderer::~AAFrameBufferRenderer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteFramebuffers(1, &intermediateFBO);
    glDeleteBuffers(1, &screenTexture);
    glDeleteFramebuffers(1, &mscat);
}

void cqsp::engine::AAFrameBufferRenderer::InitTexture(int width, int height) {
    this->width = width;
    this->height = height;
    GenerateFrameBuffer(framebuffer);

    // create a multisampled color attachment texture
    glGenTextures(1, &mscat);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mscat);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D_MULTISAMPLE, mscat, 0);

    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::AAFrameBufferRenderer::Clear() {
    ZoneScoped;
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void cqsp::engine::AAFrameBufferRenderer::BeginDraw() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void cqsp::engine::AAFrameBufferRenderer::EndDraw() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::AAFrameBufferRenderer::Free() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteFramebuffers(1, &intermediateFBO);
    glDeleteBuffers(1, &screenTexture);
    glDeleteFramebuffers(1, &mscat);
}

void cqsp::engine::AAFrameBufferRenderer::RenderBuffer() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    buffer_shader->UseProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    mesh_output->Draw();

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void cqsp::engine::AAFrameBufferRenderer::NewFrame(const Window& window) {
    BeginDraw();
    Clear();
    EndDraw();

    // Check if window size changed, and then change the window size.
    if (window.WindowSizeChanged()) {
        // Then resize window
        Free();
        InitTexture(window.GetWindowWidth(), window.GetWindowHeight());
    }
}

using cqsp::engine::LayerRenderer;
void LayerRenderer::BeginDraw(int layer) { framebuffers[layer]->BeginDraw(); }

void LayerRenderer::EndDraw(int layer) { framebuffers[layer]->EndDraw(); }

void LayerRenderer::DrawAllLayers() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& frame : framebuffers) {
        frame->RenderBuffer();
    }
}

void LayerRenderer::NewFrame(const cqsp::engine::Window& window) {
    ZoneScoped;
    for (auto& frame : framebuffers) {
        frame->NewFrame(window);
    }
}

int LayerRenderer::GetLayerCount() { return static_cast<int>(framebuffers.size()); }

void LayerRenderer::InitFramebuffer(IFramebuffer* buffer, cqsp::asset::ShaderProgram_t shader,
 const cqsp::engine::Window& window) {
    // Initialize pane
    buffer->InitTexture(window.GetWindowWidth(), window.GetWindowHeight());
    buffer->SetMesh(engine::primitive::MakeTexturedPaneMesh(true));
    buffer->SetShader(std::move(shader));
}
