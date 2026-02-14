/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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
#include "engine/graphics/shader.h"

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "engine/enginelogger.h"

namespace cqsp::asset {
unsigned int LoadShaderData(const std::string& code, int type) {
    unsigned int shader;

    shader = glCreateShader(type);
    const char* shaderC = code.c_str();
    glShaderSource(shader, 1, &shaderC, NULL);
    glCompileShader(shader);

    // Check compile suceess
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        std::string error = GetErrorLog(shader);
        // Throw exception or something
        // Bad idea, so:
        // FIXME(EhWhoAmI): Don't use throw
        // Output
        glDeleteShader(shader);
        throw(std::runtime_error(error));
    }
    return shader;
}

unsigned int MakeShaderProgram(int vertex, int fragment, int geometry) {
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometry != -1) {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);
    return ID;
}

std::string GetErrorLog(unsigned int shader) {
    GLint error_max_len = 1024;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_max_len);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(error_max_len);
    glGetShaderInfoLog(shader, error_max_len, &error_max_len, errorLog.data());

    std::string s(errorLog.begin(), errorLog.end());
    return s;
}

void ShaderProgram::setBool(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<int>(value));
}

void ShaderProgram::setInt(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

void ShaderProgram::setFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(program, name.c_str()), static_cast<GLfloat>(value));
}

void ShaderProgram::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

void ShaderProgram::setVec2(const std::string& name, float x, float y) {
    glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
}

void ShaderProgram::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

void ShaderProgram::setVec3(const std::string& name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
}

void ShaderProgram::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

void ShaderProgram::setVec4(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(program, name.c_str()), x, y, z, w);
}

void ShaderProgram::setMat2(const std::string& name, const glm::mat2& mat) {
    glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setMat3(const std::string& name, const glm::mat3& mat) {
    glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::Set(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<int>(value));
}

void ShaderProgram::Set(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

void ShaderProgram::Set(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(program, name.c_str()), static_cast<GLfloat>(value));
}

void ShaderProgram::Set(const std::string& name, const glm::vec2& value) {
    glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

void ShaderProgram::Set(const std::string& name, float x, float y) {
    glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
}

void ShaderProgram::Set(const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

void ShaderProgram::Set(const std::string& name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
}

void ShaderProgram::Set(const std::string& name, const glm::vec4& value) {
    glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

void ShaderProgram::Set(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(program, name.c_str()), x, y, z, w);
}

void ShaderProgram::Set(const std::string& name, const glm::mat2& mat) {
    glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::Set(const std::string& name, const glm::mat3& mat) {
    glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::Set(const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::UseProgram() { glUseProgram(program); }

void ShaderProgram::SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    UseProgram();
    setMat4("model", model);
    setMat4("view", view);
    setMat4("projection", projection);
}

ShaderProgram::ShaderProgram() { program = -1; }

ShaderProgram::ShaderProgram(const Shader& vert, const Shader& frag) {
    assert(vert.shader_type == ShaderType::VERT);
    assert(frag.shader_type == ShaderType::FRAG);
    program = MakeShaderProgram(vert.id, frag.id);
}

ShaderProgram::ShaderProgram(const Shader& vert, const Shader& frag, const Shader& geom) {
    assert(vert.shader_type == ShaderType::VERT);
    assert(frag.shader_type == ShaderType::FRAG);
    assert(geom.shader_type == ShaderType::GEOM);
    program = MakeShaderProgram(vert.id, frag.id, geom.id);
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(program); }

Shader::Shader(const std::string& code, ShaderType type) : id(0), shader_type(type) {
    int gl_shader_type = GL_FRAGMENT_SHADER;
    switch (shader_type) {
        case ShaderType::FRAG:
            gl_shader_type = GL_FRAGMENT_SHADER;
            break;
        case ShaderType::VERT:
            gl_shader_type = GL_VERTEX_SHADER;
            break;
        case ShaderType::GEOM:
            gl_shader_type = GL_GEOMETRY_SHADER;
            break;
        case ShaderType::NONE:
            break;
    }
    id = glCreateShader(gl_shader_type);
    const char* shader_char = code.c_str();
    glShaderSource(id, 1, &shader_char, NULL);
    glCompileShader(id);
    // Check compile suceess
    GLint isCompiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        std::string error = GetErrorLog(id);
        // Throw exception or something
        // Bad idea, so:
        // FIXME(EhWhoAmI): Don't use throw
        // Output
        glDeleteShader(id);
        throw(std::runtime_error(error));
    }
}

void Shader::operator()(const std::string& code, ShaderType type) {
    shader_type = type;
    int gl_shader_type = GL_FRAGMENT_SHADER;
    switch (shader_type) {
        case ShaderType::FRAG:
            gl_shader_type = GL_FRAGMENT_SHADER;
            break;
        case ShaderType::VERT:
            gl_shader_type = GL_VERTEX_SHADER;
            break;
        case ShaderType::GEOM:
            gl_shader_type = GL_GEOMETRY_SHADER;
            break;
        case ShaderType::NONE:
            break;
    }
    id = glCreateShader(gl_shader_type);
    const char* shader_char = code.c_str();
    glShaderSource(id, 1, &shader_char, NULL);
    glCompileShader(id);
    // Check compile suceess
    GLint isCompiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        std::string error = GetErrorLog(id);
        // Throw exception or something
        // Bad idea, so:
        // FIXME(EhWhoAmI): Don't use throw
        // Output
        glDeleteShader(id);
        throw(std::runtime_error(error));
    }
}

Shader::~Shader() {
    if (id != 0) {
        glDeleteShader(id);
    }
}
namespace {
GLenum GetUniformType(GLuint program, const char* name) {
    GLuint in[1];
    glGetUniformIndices(program, 1, &name, in);

    GLint location = *in;
    ENGINE_LOG_TRACE("Program {}: {} is {}", program, name, location);
    if (location != -1) {
        GLint size;
        GLenum type;
        GLsizei length;
        GLchar name[256];
        glGetActiveUniform(program, *in, 255, &length, &size, &type, name);
        ENGINE_LOG_TRACE("{} {}", location, name);
        return type;
    } else {
        return GL_INVALID_ENUM;
    }
}

#if DEBUG_OPENGL
// We may need this in the future if we have to debug uniforms
std::map<GLenum, std::string> type_set {{GL_INVALID_ENUM, "invalid"},
                                        {GL_FLOAT, "float"},
                                        {GL_FLOAT_VEC2, "vec2"},
                                        {GL_FLOAT_VEC3, "vec3"},
                                        {GL_FLOAT_VEC4, "vec4"},
                                        {GL_DOUBLE, "double"},
                                        {GL_DOUBLE_VEC2, "dvec2"},
                                        {GL_DOUBLE_VEC3, "dvec3"},
                                        {GL_DOUBLE_VEC4, "dvec4"},
                                        {GL_INT, "int"},
                                        {GL_INT_VEC2, "ivec2"},
                                        {GL_INT_VEC3, "ivec3"},
                                        {GL_INT_VEC4, "ivec4"},
                                        {GL_UNSIGNED_INT, "unsigned int"},
                                        {GL_UNSIGNED_INT_VEC2, "uvec2"},
                                        {GL_UNSIGNED_INT_VEC3, "uvec3"},
                                        {GL_UNSIGNED_INT_VEC4, "uvec4"},
                                        {GL_BOOL, "bool"},
                                        {GL_BOOL_VEC2, "bvec2"},
                                        {GL_BOOL_VEC3, "bvec3"},
                                        {GL_BOOL_VEC4, "bvec4"},
                                        {GL_FLOAT_MAT2, "mat2"},
                                        {GL_FLOAT_MAT3, "mat3"},
                                        {GL_FLOAT_MAT4, "mat4"},
                                        {GL_FLOAT_MAT2x3, "mat2x3"},
                                        {GL_FLOAT_MAT2x4, "mat2x4"},
                                        {GL_FLOAT_MAT3x2, "mat3x2"},
                                        {GL_FLOAT_MAT3x4, "mat3x4"},
                                        {GL_FLOAT_MAT4x2, "mat4x2"},
                                        {GL_FLOAT_MAT4x3, "mat4x3"},
                                        {GL_DOUBLE_MAT2, "dmat2"},
                                        {GL_DOUBLE_MAT3, "dmat3"},
                                        {GL_DOUBLE_MAT4, "dmat4"},
                                        {GL_DOUBLE_MAT2x3, "dmat2x3"},
                                        {GL_DOUBLE_MAT2x4, "dmat2x4"},
                                        {GL_DOUBLE_MAT3x2, "dmat3x2"},
                                        {GL_DOUBLE_MAT3x4, "dmat3x4"},
                                        {GL_DOUBLE_MAT4x2, "dmat4x2"},
                                        {GL_DOUBLE_MAT4x3, "dmat4x3"},
                                        {GL_SAMPLER_1D, "sampler1D"},
                                        {GL_SAMPLER_2D, "sampler2D"},
                                        {GL_SAMPLER_3D, "sampler3D"},
                                        {GL_SAMPLER_CUBE, "samplerCube"},
                                        {GL_SAMPLER_1D_SHADOW, "sampler1DShadow"},
                                        {GL_SAMPLER_2D_SHADOW, "sampler2DShadow"},
                                        {GL_SAMPLER_1D_ARRAY, "sampler1DArray"},
                                        {GL_SAMPLER_2D_ARRAY, "sampler2DArray"},
                                        {GL_SAMPLER_1D_ARRAY_SHADOW, "sampler1DArrayShadow"},
                                        {GL_SAMPLER_2D_ARRAY_SHADOW, "sampler2DArrayShadow"},
                                        {GL_SAMPLER_2D_MULTISAMPLE, "sampler2DMS"},
                                        {GL_SAMPLER_2D_MULTISAMPLE_ARRAY, "sampler2DMSArray"},
                                        {GL_SAMPLER_CUBE_SHADOW, "samplerCubeShadow"},
                                        {GL_SAMPLER_BUFFER, "samplerBuffer"},
                                        {GL_SAMPLER_2D_RECT, "sampler2DRect"},
                                        {GL_SAMPLER_2D_RECT_SHADOW, "sampler2DRectShadow"},
                                        {GL_INT_SAMPLER_1D, "isampler1D"},
                                        {GL_INT_SAMPLER_2D, "isampler2D"},
                                        {GL_INT_SAMPLER_3D, "isampler3D"},
                                        {GL_INT_SAMPLER_CUBE, "isamplerCube"},
                                        {GL_INT_SAMPLER_1D_ARRAY, "isampler1DArray"},
                                        {GL_INT_SAMPLER_2D_ARRAY, "isampler2DArray"},
                                        {GL_INT_SAMPLER_2D_MULTISAMPLE, "isampler2DMS"},
                                        {GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, "isampler2DMSArray"},
                                        {GL_INT_SAMPLER_BUFFER, "isamplerBuffer"},
                                        {GL_INT_SAMPLER_2D_RECT, "isampler2DRect"},
                                        {GL_UNSIGNED_INT_SAMPLER_1D, "usampler1D"},
                                        {GL_UNSIGNED_INT_SAMPLER_2D, "usampler2D"},
                                        {GL_UNSIGNED_INT_SAMPLER_3D, "usampler3D"},
                                        {GL_UNSIGNED_INT_SAMPLER_CUBE, "usamplerCube"},
                                        {GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, "usampler2DArray"},
                                        {GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, "usampler2DArray"},
                                        {GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, "usampler2DMS"},
                                        {GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, "usampler2DMSArray"},
                                        {GL_UNSIGNED_INT_SAMPLER_BUFFER, "usamplerBuffer"},
                                        {GL_UNSIGNED_INT_SAMPLER_2D_RECT, "usampler2DRect"},
                                        {GL_IMAGE_1D, "image1D"},
                                        {GL_IMAGE_2D, "image2D"},
                                        {GL_IMAGE_3D, "image3D"},
                                        {GL_IMAGE_2D_RECT, "image2DRect"},
                                        {GL_IMAGE_CUBE, "imageCube"},
                                        {GL_IMAGE_BUFFER, "imageBuffer"},
                                        {GL_IMAGE_1D_ARRAY, "image1DArray"},
                                        {GL_IMAGE_2D_ARRAY, "image2DArray"},
                                        {GL_IMAGE_2D_MULTISAMPLE, "image2DMS"},
                                        {GL_IMAGE_2D_MULTISAMPLE_ARRAY, "image2DMSArray"},
                                        {GL_INT_IMAGE_1D, "iimage1D"},
                                        {GL_INT_IMAGE_2D, "iimage2D"},
                                        {GL_INT_IMAGE_3D, "iimage3D"},
                                        {GL_INT_IMAGE_2D_RECT, "iimage2DRect"},
                                        {GL_INT_IMAGE_CUBE, "iimageCube"},
                                        {GL_INT_IMAGE_BUFFER, "iimageBuffer"},
                                        {GL_INT_IMAGE_1D_ARRAY, "iimage1DArray"},
                                        {GL_INT_IMAGE_2D_ARRAY, "iimage2DArray"},
                                        {GL_INT_IMAGE_2D_MULTISAMPLE, "iimage2DMS"},
                                        {GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY, "iimage2DMSArray"},
                                        {GL_UNSIGNED_INT_IMAGE_1D, "uimage1D"},
                                        {GL_UNSIGNED_INT_IMAGE_2D, "uimage2D"},
                                        {GL_UNSIGNED_INT_IMAGE_3D, "uimage3D"},
                                        {GL_UNSIGNED_INT_IMAGE_2D_RECT, "uimage2DRect"},
                                        {GL_UNSIGNED_INT_IMAGE_CUBE, "uimageCube"},
                                        {GL_UNSIGNED_INT_IMAGE_BUFFER, "uimageBuffer"},
                                        {GL_UNSIGNED_INT_IMAGE_1D_ARRAY, "uimage1DArray"},
                                        {GL_UNSIGNED_INT_IMAGE_2D_ARRAY, "uimage2DArray"},
                                        {GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE, "uimage2DMS"},
                                        {GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY, "uimage2DMSArray"},
                                        {GL_UNSIGNED_INT_ATOMIC_COUNTER, "atomic_uint"}};

void eTB_GLSL__print_uniforms(GLuint program) {
    GLint uniform_count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

    GLchar name[256];

    for (GLint i = 0; i < uniform_count; i++) {
        memset(name, '\0', 256);
        GLint size;
        GLenum type;

        glGetActiveUniform(program, i, 255, NULL, &size, &type, name);

        GLint location = glGetUniformLocation(program, name);

        auto& type_name = type_set[type];

        if (size > 1) {
            ENGINE_LOG_INFO("Uniform {} (loc={}):\t{} {} <Size: {}>", i, location, type_name, name, size);
        } else {
            ENGINE_LOG_INFO("Uniform {} (loc={}):\t{} {}", i, location, type_name, name);
        }
    }
}

#define debug_uniforms(program) eTB_GLSL__print_uniforms(program)
#else
#define debug_uniforms(program) (void)0
#endif
std::map<Hjson::Type, std::string> hjson_type_set {
    {Hjson::Type::Undefined, "Undefined"}, {Hjson::Type::Null, "Null"},   {Hjson::Type::Bool, "Bool"},
    {Hjson::Type::Double, "Double"},       {Hjson::Type::Int64, "Int64"}, {Hjson::Type::String, "String"},
    {Hjson::Type::Vector, "Vector"},       {Hjson::Type::Map, "Map"}};
}  // namespace

void ShaderDefinition::SetShaderUniform(ShaderProgram_t& shader, std::pair<const std::string, Hjson::Value>& value) {
    GLenum type = GetUniformType(shader->program, value.first.c_str());
    switch (type) {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_1D:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_BUFFER:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_TEXTURE_BUFFER:
        case GL_INT:
            // Type has to be int
            if (value.second.type() != Hjson::Type::Int64) {
                ENGINE_LOG_WARN("Uniform {} is not type int, it is {}", value.first,
                                hjson_type_set[value.second.type()]);
                break;
            }
            shader->Set(value.first, (int)value.second.to_int64());
            break;
        case GL_FLOAT_VEC2:
            if (value.second.type() != Hjson::Type::Vector && value.second.size() != 2) {
                ENGINE_LOG_WARN("Uniform {} is not type vec2, it is {}", value.first,
                                hjson_type_set[value.second.type()]);
                break;
            }
            shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double());
            break;
        case GL_FLOAT_VEC3:
            if (value.second.type() != Hjson::Type::Vector && value.second.size() != 3) {
                ENGINE_LOG_WARN("Uniform {} is not type vec3, it is {}", value.first,
                                hjson_type_set[value.second.type()]);
                break;
            }
            shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double(),
                        (float)value.second[2].to_double());
            break;
        case GL_FLOAT_VEC4:
            if (value.second.type() != Hjson::Type::Vector && value.second.size() != 4) {
                ENGINE_LOG_WARN("Uniform {} is not type vec4, it is {}", value.first,
                                hjson_type_set[value.second.type()]);
                break;
            }
            shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double(),
                        (float)value.second[2].to_double(), (float)value.second[3].to_double());
            break;
        case GL_FLOAT:
            if (value.second.type() != Hjson::Type::Double && value.second.type() != Hjson::Type::Int64) {
                ENGINE_LOG_WARN("Uniform {} is not type float, it is {}", value.first,
                                hjson_type_set[value.second.type()]);
                break;
            }
            shader->Set(value.first, (float)value.second.to_double());
            break;
        case GL_BOOL:
            if (value.second.type() != Hjson::Type::Bool) {
                ENGINE_LOG_WARN("Uniform {} is not type bool, it is {}", value.first,
                                hjson_type_set[value.second.type()]);
                break;
            }
            shader->Set(value.first, (bool)value.second);
            break;
        default:
            // We don't support matrices yet
            break;
    }
}

ShaderProgram_t ShaderDefinition::MakeShader() {
    Shader vert_shader(vert, ShaderType::VERT);
    Shader frag_shader(frag, ShaderType::FRAG);

    // Create the shader
    ShaderProgram_t shader = nullptr;
    if (!geometry.empty()) {
        Shader geom_shader(geometry, ShaderType::GEOM);
        // Add to the shader
        shader = MakeShaderProgram(vert_shader, frag_shader, geom_shader);
    } else {
        shader = MakeShaderProgram(vert_shader, frag_shader);
    }
    // Initial values
    shader->UseProgram();
    debug_uniforms(shader->program);

    for (auto value : uniforms) {
        SetShaderUniform(shader, value);
    }
    return shader;
}

void ShaderProgram::bindTexture(int texture, unsigned int texture_id) {
    glActiveTexture(GL_TEXTURE0 + texture);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}
}  // namespace cqsp::asset