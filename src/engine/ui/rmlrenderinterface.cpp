#include "rmlrenderinterface.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <glm/gtx/transform.hpp>

#include "common/util/logging.h"
#include "engine/application.h"
#include "engine/graphics/mesh.h"

class RmlUiRendererGeometryHandler {
 public:
    GLuint VAO, VBO, EBO;
    int num_vertices;
    int num_indices;
    cqsp::asset::Texture* texture;

    RmlUiRendererGeometryHandler()
        : VAO(0),
          VBO(0),
          EBO(0),
          num_vertices(0),
          num_indices(0),
          texture(0) {}

    ~RmlUiRendererGeometryHandler() {
        if (VAO) {
            glDeleteVertexArrays(1, &VAO);
        }

        if (VBO) {
            glDeleteBuffers(1, &VBO);
        }

        if (EBO) {
            glDeleteBuffers(1, &EBO);
        }
        VAO = VBO = EBO = 0;
    }
};

class Vertex {
    glm::vec2 pos;
    glm::vec2 tex_coord;
    glm::vec4 color;
};

cqsp::engine::CQSPRenderInterface::CQSPRenderInterface(Application& _app) : app(_app) {
    try {
    vert_shader = cqsp::asset::Shader(R"(
#version 330 core
layout (location = 0) in vec2 iPos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 iTexCoord;

uniform mat4 model;
uniform mat4 projection;
out vec4 place_color;
out vec2 TexCoord;
void main() {
    gl_Position = projection * model * vec4(iPos, 0.0, 1.0);
    // Convert color because rmlui gives it in bytes, while opengl needs it to be in floats
    // This is probabluy slow because division is slow, but this is easy.
    place_color = vec4(color/255.0);
    TexCoord = vec2(iTexCoord.x, iTexCoord.y);
}
    )", cqsp::asset::ShaderType::VERT);
        SPDLOG_INFO("Frag shader");
    frag_shader = cqsp::asset::Shader(R"(
#version 330 core
out vec4 FragColor;
in vec4 place_color;
in vec2 TexCoord;
void main()
{
    FragColor = place_color;//vec4(texture(texture1, TexCoord).rgb, 1);
}
    )", cqsp::asset::ShaderType::FRAG);
        texture_frag_shader =
            cqsp::asset::Shader(R"(
#version 330 core
out vec4 FragColor;
in vec4 place_color;
in vec2 TexCoord;
uniform sampler2D texture1;
void main()
{
    FragColor = vec4(texture(texture1, TexCoord).rgba) * vec4(place_color.rgb, 1);
}
    )", cqsp::asset::ShaderType::FRAG);

    color_shader = asset::MakeShaderProgram(vert_shader, frag_shader);
    texture_shader = asset::MakeShaderProgram(vert_shader, texture_frag_shader);
    texture_shader->UseProgram();
    texture_shader->setInt("texture1", 0);
    } catch (const std::runtime_error& ex) {
        SPDLOG_WARN("{}", ex.what());
    }
}

void cqsp::engine::CQSPRenderInterface::RenderGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture, const Rml::Vector2f& translation) {
    SPDLOG_INFO("Rendering geom");
}

Rml::CompiledGeometryHandle cqsp::engine::CQSPRenderInterface::CompileGeometry(
    Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
    Rml::TextureHandle texture) {
    RmlUiRendererGeometryHandler* geom = new RmlUiRendererGeometryHandler();

    // Create the vertex
    geom->num_vertices = num_vertices;
    geom->num_indices = num_indices;
    geom->texture = (cqsp::asset::Texture*) texture;
    glGenVertexArrays(1, &geom->VAO);
    glGenBuffers(1, &geom->VBO);
    glGenBuffers(1, &geom->EBO);

    glBindVertexArray(geom->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geom->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Rml::Vertex) * num_vertices, vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indices, indices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Rml::Vertex),
                          reinterpret_cast<void*>(sizeof(Rml::Vector2f)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex),
        reinterpret_cast<void*>(sizeof(Rml::Vector2f) + sizeof(Rml::Colourb)));
    glEnableVertexAttribArray(2);

    return (Rml::CompiledGeometryHandle) geom;
}

void cqsp::engine::CQSPRenderInterface::RenderCompiledGeometry(
    Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) {
    RmlUiRendererGeometryHandler* geom = (RmlUiRendererGeometryHandler*)geometry;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    asset::ShaderProgram* shader = nullptr;
    // Draw the geometry
    if (geom->texture) {
        texture_shader->UseProgram();
        shader = texture_shader.get();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, geom->texture->id);
    } else {
        color_shader->UseProgram();
        shader = color_shader.get();
    }
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(translation.x, translation.y, 0.0f));
    shader->Set("projection", app.Get2DProj());
    shader->Set("model", model);

    glBindVertexArray(geom->VAO);
    glDrawElements(GL_TRIANGLES, geom->num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void cqsp::engine::CQSPRenderInterface::ReleaseCompiledGeometry(
    Rml::CompiledGeometryHandle geometry) {
    delete (Rml::CompiledGeometryHandle*) geometry;
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
    glScissor(x, app.GetWindowHeight() - (y + height), width, height);
}

bool cqsp::engine::CQSPRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions,
    const Rml::String& source) {
    // Load the texture from the file
    SPDLOG_INFO("Loading image {}", source);
    // Open file and do things
    cqsp::asset::Texture* texture = new cqsp::asset::Texture();

    int width, height, components;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data2 = stbi_load(source.c_str(), &width, &height, &components, 0);
    if (!data2) {
        return false;
    }
    asset::TextureLoadingOptions options;
    // Read file
    cqsp::asset::CreateTexture(*texture, data2, width, height, components, options);
    // Dump image
    stbi_image_free(data2);
    texture_handle = (Rml::TextureHandle)texture;
    return true;
}

bool cqsp::engine::CQSPRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source,
    const Rml::Vector2i& source_dimensions) {
    cqsp::asset::Texture* texture = new cqsp::asset::Texture();
    // Generate opengl texture
    // Cast the source to things
    cqsp::asset::TextureLoadingOptions options;
    cqsp::asset::CreateTexture(*texture, (unsigned char*) (source),
        source_dimensions.x, source_dimensions.y, 4, options);
    texture_handle = (Rml::TextureHandle)texture;
    return true;
}

void cqsp::engine::CQSPRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
    delete (cqsp::asset::Texture*) texture;
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
