/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <istream>
#include <string>

#include <glm/glm.hpp>

#include "engine/asset.h"

namespace conquerspace {
namespace asset {
enum class ShaderType {VERT, FRAG, GEOM};

class Shader : public Asset {
 public:
    unsigned int id;
};

class ShaderProgram {
 public:
    ShaderProgram();
    ShaderProgram(Shader& vert, Shader& frag);

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, int value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec2(const std::string& name, float x, float y);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec3(const std::string& name, float x, float y, float z);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat);
    void setMat3(const std::string& name, const glm::mat3& mat);
    void setMat4(const std::string& name, const glm::mat4& mat);

    void UseProgram();
    unsigned int program;

    operator unsigned int() const { return program; }
};

// Set of utility functions that load shaders
unsigned int LoadShader(std::string& shader, int type);

unsigned int MakeShaderProgram(int vertex, int fragment);

std::string GetErrorLog(unsigned int shader);
}  // namespace asset
}  // namespace conquerspace
