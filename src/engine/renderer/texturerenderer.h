/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "engine/renderer/renderer.h"

namespace conquerspace {
namespace engine {
class TextureRenderer {
 public:
    glm::mat4 projection = glm::mat4(1.0);
    glm::mat4 view = glm::mat4(1.0);
    std::vector<BasicRendererObject> renderables;
    conquerspace::asset::ShaderProgram* buffer_shader;
    conquerspace::engine::Mesh* mesh_output;
    unsigned int framebuffer = 0;
    void Draw();
    void RenderBuffer();
};

class FramebufferRenderer {
 public:
    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    conquerspace::asset::ShaderProgram buffer_shader;
    conquerspace::engine::Mesh mesh_output;
    void InitTexture();
    void Clear();
    void BeginDraw();
    void EndDraw();
    void RenderBuffer();
};


class AAFrameBufferRenderer {
    unsigned int framebuffer;
    unsigned int intermediateFBO;
    unsigned int screenTexture;
    unsigned int textureColorBufferMultiSampled;
    conquerspace::asset::ShaderProgram buffer_shader;
    conquerspace::engine::Mesh mesh_output;
    void InitTexture();
    void Clear();
    void BeginDraw();
    void EndDraw();
    void RenderBuffer();
};
}  // namespace engine
}  // namespace conquerspace