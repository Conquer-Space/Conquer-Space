#include "rmlrenderinterface.h"

#include <glad/glad.h>
#include <gl/GL.h>

#include "engine/application.h"
#include "engine/graphics/mesh.h"

#include "common/util/logging.h"

cqsp::engine::CQSPRenderInterface::CQSPRenderInterface(Application& _app) : app(_app) {
    renderer = std::make_unique<cqsp::engine::Renderer2D>(nullptr);
    // Initialize shaders
    std::string pane = R"r(
#version 330 core

layout (location = 0) in vec2 iPos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 iTexCoord;

uniform mat4 model;
uniform mat4 projection;

out vec2 TexCoord;

void main()
{
    gl_Position = projection * model * vec4(iPos, 0.0, 1.0);
    TexCoord = vec2(iTexCoord.x, iTexCoord.y);
}
)r";

    std::string frag = R"r(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
)r";
    // Create shader
    cqsp::asset::Shader shader_pane = cqsp::asset::LoadShader(pane, cqsp::asset::ShaderType::VERT);
    cqsp::asset::Shader shader_frag = cqsp::asset::LoadShader(frag, cqsp::asset::ShaderType::FRAG);
    cqsp::asset::MakeShaderProgram(shader_pane, shader_frag);
}

void cqsp::engine::CQSPRenderInterface::RenderGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture, const Rml::Vector2f& translation) {
    // Do some math and render
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
    SPDLOG_INFO("Made geometry");
    return reinterpret_cast<Rml::CompiledGeometryHandle>(m);
}

void cqsp::engine::CQSPRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) {
    // A mesh
    // Set transformation too
    auto renderable = reinterpret_cast<cqsp::engine::Renderable*>(geometry);
    renderer->DrawTexturedSprite(renderable, glm::vec2(translation.x, translation.y));
}

void cqsp::engine::CQSPRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) {
    // Delete the renderable
    auto renderable = reinterpret_cast<cqsp::engine::Renderable*>(geometry);
    cqsp::engine::Mesh::Destroy(*renderable->mesh);
}

void cqsp::engine::CQSPRenderInterface::EnableScissorRegion(bool enable) {
    /*
    if (enable) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }*/
}

void cqsp::engine::CQSPRenderInterface::SetScissorRegion(int x, int y,
                                                         int width,
                                                         int height) {
    // Broken because glscissor does this: window-space lower-left position of the scissor box, and width and height define the size of the rectangle.
    // And rmlui does this: The top-most pixel to be rendered. All pixels to the top of this should be clipped.
    SPDLOG_INFO("Scissor");
    //glScissor(x, y, width, height);
}

bool cqsp::engine::CQSPRenderInterface::LoadTexture(
    Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions,
    const Rml::String& source) {
    return false;
}

bool cqsp::engine::CQSPRenderInterface::GenerateTexture(
    Rml::TextureHandle& texture_handle, const Rml::byte* source,
    const Rml::Vector2i& source_dimensions) {
    cqsp::asset::Texture* texture = new cqsp::asset::Texture();
    // Generate opengl texture
    // Cast the source to things
    cqsp::asset::TextureLoadingOptions options;
    cqsp::asset::LoadTexture(*texture, reinterpret_cast<const unsigned char*>(source), 4, source_dimensions.x, source_dimensions.y, options);
    texture_handle = (Rml::TextureHandle)texture;
    cqsp::asset::SaveImage("test.png", source_dimensions.x, source_dimensions.y,
                           4, reinterpret_cast<const unsigned char*>(source));
    return true;
}

void cqsp::engine::CQSPRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
    reinterpret_cast<cqsp::asset::Texture*>(texture)->Delete();
}

// Ignore for now
void cqsp::engine::CQSPRenderInterface::SetTransform(const Rml::Matrix4f* transform) {
}

void cqsp::engine::CQSPRenderInterface::PrepareRenderBuffer() {
    renderer->SetProjection(app.Get2DProj());
}

void cqsp::engine::CQSPRenderInterface::PresentRenderBuffer() {
}
