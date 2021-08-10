#include "texturerenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include "common/util/profiler.h"

void cqsp::engine::TextureRenderer::Draw() {
    if (framebuffer == 0) {
        glGenFramebuffers(1, &framebuffer);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // make sure we clear the framebuffer's content
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto renderable : renderables) {
        renderable->shaderProgram->UseProgram();
        renderable->shaderProgram->setMat4("model", renderable->model);
        renderable->shaderProgram->setMat4("view", view);
        renderable->shaderProgram->setMat4("projection", projection);
        int i = 0;
        for (std::vector<cqsp::asset::Texture*>::iterator it = renderable->textures.begin();
                                                    it != renderable->textures.end(); ++it) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture((*it)->texture_type, (*it)->id);
            i++;
        }

        glBindVertexArray(renderable->mesh->VAO);
        if (renderable->mesh->buffer_type == 1) {
            glDrawElements(renderable->mesh->RenderType, renderable->mesh->indicies,
                                                                        GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(renderable->mesh->RenderType, 0, renderable->mesh->indicies);
        }
        glBindVertexArray(0);

        // Reset active texture
        glActiveTexture(GL_TEXTURE0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::TextureRenderer::RenderBuffer() {
    buffer_shader->UseProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer);

    glBindVertexArray(mesh_output->VAO);
    if (mesh_output->buffer_type == 1) {
        glDrawElements(mesh_output->RenderType, mesh_output->indicies, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(mesh_output->RenderType, 0, mesh_output->indicies);
    }

    glBindVertexArray(0);

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void cqsp::engine::FramebufferRenderer::InitTexture(int width, int height) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture

    glGenTextures(1, &colorbuffer);
    glBindTexture(GL_TEXTURE_2D, colorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::FramebufferRenderer::Clear() {
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void cqsp::engine::FramebufferRenderer::BeginDraw() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void cqsp::engine::FramebufferRenderer::EndDraw() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::FramebufferRenderer::RenderBuffer() {
    buffer_shader.UseProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorbuffer);

    glBindVertexArray(mesh_output.VAO);
    if (mesh_output.buffer_type == 1) {
        glDrawElements(mesh_output.RenderType, mesh_output.indicies, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(mesh_output.RenderType, 0, mesh_output.indicies);
    }

    glBindVertexArray(0);

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void cqsp::engine::FramebufferRenderer::Free() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteBuffers(1, &colorbuffer);
}

void cqsp::engine::FramebufferRenderer::NewFrame(Window& window) {
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

void cqsp::engine::AAFrameBufferRenderer::InitTexture(int width,
                                                              int height) {
    this->width = width;
    this->height = height;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        spdlog::info("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        spdlog::info("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cqsp::engine::AAFrameBufferRenderer::Clear() {
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
    glDeleteFramebuffers(1, &textureColorBufferMultiSampled);
}

void cqsp::engine::AAFrameBufferRenderer::RenderBuffer() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT,  GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    buffer_shader.UseProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    glBindVertexArray(mesh_output.VAO);
    if (mesh_output.buffer_type == 1) {
        glDrawElements(mesh_output.RenderType, mesh_output.indicies, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(mesh_output.RenderType, 0, mesh_output.indicies);
    }

    glBindVertexArray(0);

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void cqsp::engine::AAFrameBufferRenderer::NewFrame(Window& window) {
    BeginDraw();
    Clear();
    EndDraw();

    // Remake frame
    // Check if window size changed, and then change the window size.
    if (window.WindowSizeChanged()) {
        // Then resize window
        Free();
        InitTexture(window.GetWindowWidth(), window.GetWindowHeight());
    }
}
