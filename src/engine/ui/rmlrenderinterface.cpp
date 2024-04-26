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
#include "engine/ui/rmlrenderinterface.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "common/util/logging.h"
#include "engine/application.h"
#include "engine/graphics/mesh.h"

using cqsp::engine::CQSPRenderInterface;

namespace cqsp::engine {
class RmlUiRendererGeometryHandler {
 public:
    GLuint VAO, VBO, EBO;
    int num_vertices;
    int num_indices;
    cqsp::asset::Texture* texture;

    RmlUiRendererGeometryHandler() : VAO(0), VBO(0), EBO(0), num_vertices(0), num_indices(0), texture(0) {}

    ~RmlUiRendererGeometryHandler() {
        if (VAO != 0u) {
            glDeleteVertexArrays(1, &VAO);
        }

        if (VBO != 0u) {
            glDeleteBuffers(1, &VBO);
        }

        if (EBO != 0u) {
            glDeleteBuffers(1, &EBO);
        }
        VAO = VBO = EBO = 0;
    }
};
}  // namespace cqsp::engine

CQSPRenderInterface::CQSPRenderInterface(Application& _app) : app(_app) {
    logger = spdlog::get("RmlUi");
    if (logger == nullptr) {
        logger = spdlog::default_logger();
    }
    SPDLOG_LOGGER_INFO(logger, "Initializing RmlUi version {}", Rml::GetVersion());

    // Semi sketchy code to load the shaders. It will be preferable to load it from a file, but
    // since this is rather crucial to the running the game,
    // this will likely be here until we find an alternative solution.
    try {
        vert_shader(R"(
#version 330 core
layout (location = 0) in vec2 iPos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 iTexCoord;

uniform mat4 model;
uniform mat4 projection;
out vec4 place_color;
out vec2 TexCoord;
void main() {
    gl_Position = projection * model * vec4(iPos, 1.0, 1.0);
    // Convert color because rmlui gives it in bytes, while opengl needs it to be in floats
    // This is probably slow because division is slow, but this is easy.
    place_color = vec4(color/255.0);
    TexCoord = vec2(iTexCoord.x, iTexCoord.y);
}
    )",
                    cqsp::asset::ShaderType::VERT);
        SPDLOG_LOGGER_INFO(logger, "Loaded rmlui vert shader");
        frag_shader(R"(
#version 330 core
out vec4 FragColor;
in vec4 place_color;
in vec2 TexCoord;
void main()
{
    FragColor = place_color;
})",
                    cqsp::asset::ShaderType::FRAG);

        SPDLOG_LOGGER_INFO(logger, "Loaded rmlui frag shader");
        texture_frag_shader(R"(
#version 330 core
out vec4 FragColor;
in vec4 place_color;
in vec2 TexCoord;
uniform sampler2D texture1;

void main()
{
    FragColor = vec4(texture(texture1, TexCoord).rgba) * vec4(place_color.rgba);
})",
                            cqsp::asset::ShaderType::FRAG);

        color_shader = asset::MakeShaderProgram(vert_shader, frag_shader);
        texture_shader = asset::MakeShaderProgram(vert_shader, texture_frag_shader);
        texture_shader->UseProgram();
        texture_shader->setInt("texture1", 0);
    } catch (const std::runtime_error& ex) {
        // Need something better than throwing an exception
        // TODO(EhWhoAmI): Also need to disable the game from displaying ui if this fails
        SPDLOG_LOGGER_WARN(logger, "{}", ex.what());
    }
    SPDLOG_LOGGER_INFO(logger, "Initialized RmlUi renderer");
}

void CQSPRenderInterface::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices,
                                                       int num_indices, Rml::TextureHandle texture,
                                                       const Rml::Vector2f& translation) {
    // Render the geometry
    Rml::CompiledGeometryHandle compiled = CompileGeometry(vertices, num_vertices, indices, num_indices, texture);
    if (compiled != 0) {
        RenderCompiledGeometry(compiled, translation);
        ReleaseCompiledGeometry(compiled);
    }
}

Rml::CompiledGeometryHandle CQSPRenderInterface::CompileGeometry(Rml::Vertex* vertices, int num_vertices,
                                                                               int* indices, int num_indices,
                                                                               Rml::TextureHandle texture) {
    RmlUiRendererGeometryHandler* geom = new RmlUiRendererGeometryHandler();

    // Create the vertex
    geom->num_vertices = num_vertices;
    geom->num_indices = num_indices;
    geom->texture = (cqsp::asset::Texture*)texture;  // NOLINT
    glGenVertexArrays(1, &geom->VAO);
    glGenBuffers(1, &geom->VBO);
    glGenBuffers(1, &geom->EBO);

    glBindVertexArray(geom->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, geom->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Rml::Vertex) * num_vertices, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indices, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Rml::Vertex),
                          reinterpret_cast<void*>(sizeof(Rml::Vector2f)));  // NOLINT
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex),
                          reinterpret_cast<void*>(sizeof(Rml::Vector2f) + sizeof(Rml::Colourb)));  // NOLINT
    glEnableVertexAttribArray(2);

    return reinterpret_cast<Rml::CompiledGeometryHandle>(geom);
}

void CQSPRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry,
                                                               const Rml::Vector2f& translation) {
    RmlUiRendererGeometryHandler* geom = reinterpret_cast<RmlUiRendererGeometryHandler*>(geometry);  // NOLINT
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_ALWAYS);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    asset::ShaderProgram* shader = nullptr;
    // Draw the geometry
    if (geom->texture != nullptr) {
        texture_shader->UseProgram();
        shader = texture_shader.get();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, geom->texture->id);
    } else {
        color_shader->UseProgram();
        shader = color_shader.get();
    }
    glm::mat4 model = glm::mat4(1.0f);
    model = m_transform_matrix * glm::translate(model, glm::vec3(translation.x, translation.y, 0.0f));
    // Translate
    shader->Set("projection", app.GetRmlUiProj());
    shader->Set("model", model);
    // Set translation matrix

    glBindVertexArray(geom->VAO);
    glDrawElements(GL_TRIANGLES, geom->num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CQSPRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) {
    delete (RmlUiRendererGeometryHandler*)geometry;  // NOLINT
}

void CQSPRenderInterface::EnableScissorRegion(bool enable) {
    if (enable) {
        if (!m_transform_enabled) {
            glEnable(GL_SCISSOR_TEST);
            glDisable(GL_STENCIL_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
            glEnable(GL_STENCIL_TEST);
        }
    } else {
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
    }
}

void CQSPRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
    glScissor(x, app.GetWindowHeight() - (y + height), width, height);
    // TODO(EhWhoAmI): Add stencil buffer rendering
    // Reference:
    // https://github.com/mikke89/RmlUi/blob/master/Samples/shell/src/ShellRenderInterfaceOpenGL.cpp#L120
}

bool CQSPRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle,
                                                    Rml::Vector2i& texture_dimensions, const Rml::String& source) {
    // Load the texture from the file
    SPDLOG_LOGGER_INFO(logger, "Loading image {}", source);
    // Open file and do things
    asset::Texture* texture = new asset::Texture();

    int width;
    int height;
    int components;
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data2 = stbi_load(source.c_str(), &width, &height, &components, 0);
    if (data2 == nullptr) {
        delete texture;
        return false;
    }
    asset::TextureLoadingOptions options;
    // Read file
    asset::CreateTexture(*texture, data2, width, height, components, options);
    texture_dimensions.x = width;
    texture_dimensions.y = height;
    // Dump image
    stbi_image_free(data2);
    texture_handle = (Rml::TextureHandle)texture;
    return true;
}

bool CQSPRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source,
                                                        const Rml::Vector2i& source_dimensions) {
    asset::Texture* texture = new asset::Texture();
    // Generate opengl texture
    // Cast the source to things
    asset::TextureLoadingOptions options;
    asset::CreateTexture(*texture, (unsigned char*)(source), source_dimensions.x, source_dimensions.y, 4,
                               options);
    texture_handle = (Rml::TextureHandle)texture;
    return true;
}

void CQSPRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
    delete (asset::Texture*)texture;  // NOLINT
}

void CQSPRenderInterface::SetTransform(const Rml::Matrix4f* transform) {
    m_transform_enabled = (transform != nullptr);

    if (transform == nullptr) {
        m_transform_matrix = glm::mat4(1.0);
        return;
    }
    // Super sketchy code to convert, please help, change this
    m_transform_matrix = glm::mat4 {(*transform)[0][0], (*transform)[0][1], (*transform)[0][2], (*transform)[0][3],
                                    (*transform)[1][0], (*transform)[1][1], (*transform)[1][2], (*transform)[1][3],
                                    (*transform)[2][0], (*transform)[2][1], (*transform)[2][2], (*transform)[2][3],
                                    (*transform)[3][0], (*transform)[3][1], (*transform)[3][2], (*transform)[3][3]};
}

void CQSPRenderInterface::PrepareRenderBuffer() {}

void CQSPRenderInterface::PresentRenderBuffer() {}
