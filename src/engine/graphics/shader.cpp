/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <vector>

unsigned int cqsp::asset::LoadShaderData(const std::string& code, int type) {
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

unsigned int cqsp::asset::MakeShaderProgram(int vertex, int fragment, int geometry) {
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometry != -1) {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);
    return ID;
}

std::string cqsp::asset::GetErrorLog(unsigned int shader) {
    GLint error_max_len = 1024;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_max_len);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(error_max_len);
    glGetShaderInfoLog(shader, error_max_len, &error_max_len, &errorLog[0]);

    std::string s(errorLog.begin(), errorLog.end());
    return s;
}

using cqsp::asset::ShaderProgram;

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

void ShaderProgram::UseProgram() {
    glUseProgram(program);
}

ShaderProgram::ShaderProgram() { program = -1; }

#include <spdlog/spdlog.h>

ShaderProgram::ShaderProgram(const Shader& vert, const Shader& frag) {
    assert(vert.shader_type == ShaderType::VERT);
    assert(frag.shader_type == ShaderType::FRAG);
    program = MakeShaderProgram(vert.id, frag.id);
    
}

cqsp::asset::ShaderProgram::ShaderProgram(const Shader& vert,
                                          const Shader& frag,
                                          const Shader& geom) {
    assert(vert.shader_type == ShaderType::VERT);
    assert(frag.shader_type == ShaderType::FRAG);
    assert(geom.shader_type == ShaderType::GEOM);
    program = MakeShaderProgram(vert.id, frag.id, geom.id);

}

cqsp::asset::ShaderProgram::~ShaderProgram() {
    glDeleteProgram(program);
}

cqsp::asset::Shader::Shader(const std::string& code, ShaderType type)
    : id(0), shader_type(type) {
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

void cqsp::asset::Shader::operator()(const std::string& code, ShaderType type) {
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

cqsp::asset::Shader::~Shader() {
    if (id != 0) {
        glDeleteShader(id);
    }
}

GLenum GetUniformType(int program, const char* name) {
    int location = glGetUniformLocation(program, name);
    if (location != -1) {
        GLint size;
        GLenum type;
        GLsizei length;
        glGetActiveUniform(program, location, 0, &length, &size, &type, NULL);
        return type;
    } else {
        return GL_INVALID_ENUM;
    }
}

cqsp::asset::ShaderProgram_t cqsp::asset::ShaderDefinition::MakeShader() {
    cqsp::asset::Shader vert_shader(vert, cqsp::asset::ShaderType::VERT);
    cqsp::asset::Shader frag_shader(frag, cqsp::asset::ShaderType::FRAG);
    // Create the shader
    cqsp::asset::ShaderProgram_t shader =
        cqsp::asset::MakeShaderProgram(vert_shader, frag_shader);
    // Initial values
    shader->UseProgram();
    for (auto value : uniforms) {
        int type = GetUniformType(shader->program, value.first.c_str());
        switch (type) {
            case GL_SAMPLER_1D:
            case GL_SAMPLER_2D:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_1D_SHADOW:
            case GL_SAMPLER_2D_SHADOW:
                shader->Set(value.first, (int)value.second.to_int64());
                break;
            case GL_FLOAT_VEC2:
                shader->Set(value.first, (float)value.second[0].to_double(),
                            (float)value.second[1].to_double());
                break;
            case GL_FLOAT_VEC3:
                shader->Set(value.first, (float)value.second[0].to_double(),
                            (float)value.second[1].to_double(),
                            (float)value.second[2].to_double());
                break;
            case GL_FLOAT_VEC4:
                shader->Set(value.first, (float)value.second[0].to_double(),
                            (float)value.second[1].to_double(),
                            (float)value.second[2].to_double(),
                            (float)value.second[3].to_double());
                break;
            case GL_FLOAT:
                shader->Set(value.first, (float)value.second.to_double());
                break;
            case GL_INT:
                shader->Set(value.first, (int)value.second.to_int64());
                break;
            case GL_BOOL:
                shader->Set(value.first, (bool)value.second);
                break;
            // We don't support matrices
        }
    }
    return shader;
}
