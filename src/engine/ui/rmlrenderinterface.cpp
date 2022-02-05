#include "rmlrenderinterface.h"

#include <glad/glad.h>
#include <stb_image.h>

#include "common/util/logging.h"
#include "engine/application.h"
#include "engine/graphics/mesh.h"

cqsp::engine::CQSPRenderInterface::CQSPRenderInterface(Application& _app)
    : app(_app) {

}

void cqsp::engine::CQSPRenderInterface::RenderGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture, const Rml::Vector2f& translation) {
    // Do some math and render
    SPDLOG_INFO("render geometry, we don't support it");
}

Rml::CompiledGeometryHandle cqsp::engine::CQSPRenderInterface::CompileGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture) {
    return (Rml::CompiledGeometryHandle) nullptr;
}

void cqsp::engine::CQSPRenderInterface::RenderCompiledGeometry(
    Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) {

}

void cqsp::engine::CQSPRenderInterface::ReleaseCompiledGeometry(
    Rml::CompiledGeometryHandle geometry) {
}

void cqsp::engine::CQSPRenderInterface::EnableScissorRegion(bool enable) {
    if (enable)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

void cqsp::engine::CQSPRenderInterface::SetScissorRegion(int x, int y,
                                                         int width,
                                                         int height) {
    // Broken because glscissor does this: window-space lower-left position of
    // the scissor box, and width and height define the size of the rectangle.
    // And rmlui does this: The top-most pixel to be rendered. All pixels to the
    // top of this should be clipped.
    glScissor(x, app.GetWindowWidth() - (y + height), width, height);
}

bool cqsp::engine::CQSPRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions,
    const Rml::String& source) {
    return false;
}

bool cqsp::engine::CQSPRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source,
    const Rml::Vector2i& source_dimensions) {
    return false;
}

void cqsp::engine::CQSPRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
}

// Ignore for now
void cqsp::engine::CQSPRenderInterface::SetTransform(const Rml::Matrix4f* transform) {

}

void cqsp::engine::CQSPRenderInterface::PrepareRenderBuffer() {
  //  renderer->SetProjection(app.Get2DProj());
//glDisable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void cqsp::engine::CQSPRenderInterface::PresentRenderBuffer() {}
