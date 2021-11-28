#pragma once

#include <RmlUi/Core.h>
#include <spdlog/spdlog.h>

#include "engine/engine.h"
#include "engine/graphics/shader.h"
#include "engine/renderer/renderer2d.h"


namespace cqsp::engine {
class CQSPRenderInterface : public Rml::RenderInterface {
 public:
    CQSPRenderInterface(Application& app);
    ~CQSPRenderInterface() {}
    void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices,
                   int num_indices, Rml::TextureHandle texture,
                   const Rml::Vector2f& translation) override;

    Rml::CompiledGeometryHandle CompileGeometry(
        Rml::Vertex* vertices,
                                                   int num_vertices,
                                                   int* indices,
                                                   int num_indices,
        Rml::TextureHandle texture) override;

    void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry,
        const Rml::Vector2f& translation) override;

    void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry);

    void EnableScissorRegion(bool enable) override;

    void SetScissorRegion(int x, int y, int width, int height) override;

    bool LoadTexture(Rml::TextureHandle& texture_handle,
                         Rml::Vector2i& texture_dimensions,
                             const Rml::String& source) override;

    bool GenerateTexture(Rml::TextureHandle& texture_handle,
                                 const Rml::byte* source,
        const Rml::Vector2i& source_dimensions) override;

    void ReleaseTexture(Rml::TextureHandle texture) override;

    void SetTransform(const Rml::Matrix4f* transform) override;
    void PrepareRenderBuffer();
    void PresentRenderBuffer();

    struct Image {
        int width = 0;
        int height = 0;
        int num_components = 0;
        Rml::UniquePtr<Rml::byte[]> data;
    };

 private:
    Application& app;
    std::unique_ptr<cqsp::engine::Renderer2D> renderer;
    int m_width;
    int m_height;
    bool m_transform_enabled;
};
}  // namespace cqsp::engine