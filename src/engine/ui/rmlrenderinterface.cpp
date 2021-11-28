#include "rmlrenderinterface.h"

#include <glad/glad.h>

#include <stb_image.h>

#include "engine/application.h"
#include "engine/graphics/mesh.h"

#include "common/util/logging.h"

cqsp::engine::CQSPRenderInterface::CQSPRenderInterface(Application& _app) : app(_app) {
    // Initialize shaders
    std::string pane = R"r(
#version 330 core

layout (location = 0) in vec2 iPos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 iTexCoord;

uniform mat4 model;
uniform mat4 projection;

out vec4 place_color;
out vec2 TexCoord;

void main()
{
    gl_Position = projection * model * vec4(iPos, 0.0, 1.0);
    place_color = color;
    TexCoord = vec2(iTexCoord.x, iTexCoord.y);
}
)r";

    std::string frag = R"r(
#version 330 core
out vec4 FragColor;

in vec4 place_color;
in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
    //float alpha = texture(texture1, TexCoord).a;

    FragColor = vec4(texture(texture1, TexCoord).rgb, 1);
}
)r";
    // Create shader
    cqsp::asset::Shader shader_pane = cqsp::asset::LoadShader(pane, cqsp::asset::ShaderType::VERT);
    cqsp::asset::Shader shader_frag = cqsp::asset::LoadShader(frag, cqsp::asset::ShaderType::FRAG);
    auto texture = cqsp::asset::MakeShaderProgram(shader_pane, shader_frag);
    texture->UseProgram();
    texture->Set("texture1", 0);
    renderer = std::make_unique<cqsp::engine::Renderer2D>(texture);

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
    // Return a pointer to a mesh
    Rml::Vertex* vert = vertices;
    /*
    SPDLOG_INFO("{}", num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        vert++;
        SPDLOG_INFO("{} {} ({} {} {} {}), {} {}", vert->position.x,
                    vert->position.y, vert->colour.red, vert->colour.green,
                    vert->colour.blue, vert->colour.alpha, vert->tex_coord.x,
                    vert->tex_coord.y);
    }*/
    Mesh* m = new Mesh();
    m->indicies = num_indices;
    m->mode = GL_TRIANGLES;
    glGenVertexArrays(1, &m->VAO);
    glGenBuffers(1, &m->VBO);
    glGenBuffers(1, &m->EBO);

    glBindVertexArray(m->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Rml::Vertex) * num_vertices, vertices,
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
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(sizeof(Rml::Vector2f) + sizeof(Rml::Colourb)));
    glEnableVertexAttribArray(2);

    // Need a new renderable for rmlui specifically because it doesn't contain a shader.
    cqsp::engine::Renderable* renderable = new cqsp::engine::Renderable();
    renderable->mesh = m;
    if (texture != NULL) {
        renderable->textures.push_back(reinterpret_cast<cqsp::asset::Texture*>(texture));
    }
    return reinterpret_cast<Rml::CompiledGeometryHandle>(renderable);
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
    if (renderable->mesh != nullptr) {
        cqsp::engine::Mesh::Destroy(*(renderable->mesh));
    }
}

void cqsp::engine::CQSPRenderInterface::EnableScissorRegion(bool enable) {
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

void cqsp::engine::CQSPRenderInterface::SetScissorRegion(int x, int y,
                                                         int width,
                                                         int height) {
    // Broken because glscissor does this: window-space lower-left position of the scissor box, and width and height define the size of the rectangle.
    // And rmlui does this: The top-most pixel to be rendered. All pixels to the top of this should be clipped.
    SPDLOG_INFO("Scissor {} {} {} {}", x, y, width, height);
    glScissor(x, app.GetWindowHeight() - (y + height), width, height);
}

bool cqsp::engine::CQSPRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions,
    const Rml::String& source) {
    // Load the texture from the file
    SPDLOG_INFO("Loading texture {}", source);
    // Open file and do things
    cqsp::asset::Texture* texture = new cqsp::asset::Texture();
    // Read all the input
    std::ifstream input(source, std::ios::binary);
    if (!input.good()) {
        return false;
    }
    input.seekg(0, std::ios::end);
    // Get size
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    char* data = new char[size];
    input.read(data, size);
    unsigned char* d = (unsigned char *) data;
    int width, height, components;
    unsigned char *data2 = stbi_load_from_memory(d, size, &width, &height, &components, 0);
    asset::TextureLoadingOptions options;
    // Read file
    cqsp::asset::LoadTexture(*texture, data2,
                    width,
                    height,
                    components,
                    options);
    stbi_image_free(data2);
    delete[] data;
    texture_handle = (Rml::TextureHandle) texture;
    return true;
}

bool cqsp::engine::CQSPRenderInterface::GenerateTexture(
    Rml::TextureHandle& texture_handle, const Rml::byte* source,
    const Rml::Vector2i& source_dimensions) {
    cqsp::asset::Texture* texture = new cqsp::asset::Texture();
    // Generate opengl texture
    // Cast the source to things
    cqsp::asset::TextureLoadingOptions options;
    cqsp::asset::LoadTexture(*texture, reinterpret_cast<const unsigned char*>(source), source_dimensions.x, source_dimensions.y, 4, options);
    texture_handle = (Rml::TextureHandle) texture;
    cqsp::asset::SaveImage(fmt::format("{}.png", counter).c_str(),
                           source_dimensions.x, source_dimensions.y, 4, reinterpret_cast<const unsigned char*>(source));
    counter++;
    SPDLOG_INFO("Generated texture");
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
    glDisable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void cqsp::engine::CQSPRenderInterface::PresentRenderBuffer() {
}
