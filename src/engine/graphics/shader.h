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
#pragma once

#include <istream>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "engine/asset/asset.h"

namespace cqsp {
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
    ~ShaderProgram();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    // Simpler overloaded functions so that you can just say set xxx and change the type
    // as and when you like.
    void Set(const std::string& name, bool value) const;
    void Set(const std::string& name, int value) const;
    void Set(const std::string& name, float value) const;
    void Set(const std::string& name, const glm::vec2& value) const;
    void Set(const std::string& name, float x, float y) const;
    void Set(const std::string& name, const glm::vec3& value) const;
    void Set(const std::string& name, float x, float y, float z) const;
    void Set(const std::string& name, const glm::vec4& value) const;
    void Set(const std::string& name, float x, float y, float z, float w) const;
    void Set(const std::string& name, const glm::mat2& mat) const;
    void Set(const std::string& name, const glm::mat3& mat) const;
    void Set(const std::string& name, const glm::mat4& mat) const;

    void UseProgram() const;
    unsigned int program;

    operator unsigned int() const { return program; }
};

typedef std::shared_ptr<ShaderProgram> ShaderProgram_t;
// Set of utility functions that load shaders
unsigned int LoadShaderData(std::string& shader, int type);

unsigned int MakeShaderProgram(int vertex, int fragment);

std::string GetErrorLog(unsigned int shader);
}  // namespace asset
}  // namespace cqsp
