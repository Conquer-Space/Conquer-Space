#include "rmlrenderinterface.h"

#include <glad/glad.h>
#include <gl/GL.h>

#include "engine/graphics/mesh.h"
#include "engine/graphics/renderable.h"

cqsp::engine::CQSPRenderInterface::CQSPRenderInterface(Application& app) {}

void cqsp::engine::CQSPRenderInterface::RenderGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture, const Rml::Vector2f& translation) {
}

Rml::CompiledGeometryHandle cqsp::engine::CQSPRenderInterface::CompileGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture) {
    // Return a pointer to a mesh
    Mesh* m = new Mesh();
    glGenVertexArrays(1, &m->VAO);
    glGenBuffers(1, &m->VBO);
    glGenBuffers(1, &m->EBO);

    glBindVertexArray(m->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Rml::Vertex) * num_vertices, &vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indices, indices,
                 GL_STATIC_DRAW);

    // Set attributes
    // position attribute
    int stride = sizeof(Rml::Vector2f) + sizeof(Rml::Colourb) + sizeof(Rml::Vector2f);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_BYTE, GL_FALSE, stride,
                          reinterpret_cast<void*>(sizeof(Rml::Vector2f)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(sizeof(Rml::Vector2f) + sizeof(Rml::Colourb)));
    glEnableVertexAttribArray(2);

    // Make some sort of renderable, I guess
    cqsp::engine::Renderable renderable;
    renderable.mesh = m;
    //renderable.textures.push_back(texture);
    return reinterpret_cast<Rml::CompiledGeometryHandle>(m);
}

void cqsp::engine::CQSPRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) {
    // A mesh
    // Set transformation too
    //reinterpret_cast<cqsp::engine::Renderable>(geometry);
}

void cqsp::engine::CQSPRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) {
}

void cqsp::engine::CQSPRenderInterface::EnableScissorRegion(bool enable) {
    if (enable) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void cqsp::engine::CQSPRenderInterface::SetScissorRegion(int x, int y,
                                                         int width,
                                                         int height) {
    // Broken because glscissor does this: window-space lower-left position of the scissor box, and width and height define the size of the rectangle.
    // And rmlui does this: The top-most pixel to be rendered. All pixels to the top of this should be clipped.
    glScissor(x, y, width, height);
}

bool cqsp::engine::CQSPRenderInterface::LoadTexture(
    Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions,
    const Rml::String& source) {
    return false;
}

bool cqsp::engine::CQSPRenderInterface::GenerateTexture(
    Rml::TextureHandle& texture_handle, const Rml::byte* source,
    const Rml::Vector2i& source_dimensions) {
    return false;
}

void cqsp::engine::CQSPRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
}

void cqsp::engine::CQSPRenderInterface::SetTransform(
    const Rml::Matrix4f* transform) {}
