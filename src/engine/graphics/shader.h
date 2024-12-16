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
#pragma once

#include <hjson.h>

#include <istream>
#include <memory>
#include <string>
#include <utility>

#include <glm/glm.hpp>

#include "engine/asset/asset.h"

namespace cqsp {
namespace asset {
enum class ShaderType {
    NONE,  //!< Invalid shader
    VERT,  //!< Vertex shader
    FRAG,  //!< Fragment shader
    GEOM   //!< Geometry shader
};

/// An opengl shader.
///
/// Do note that this is distinct from a shader program, and this has to be combined with either a vertex
/// or fragmenet shader counterpart to be used in @ref ShaderProgram
class Shader : public Asset {
 public:
    Shader() = default;
    /// Constructs a shader from the code provided.
    Shader(const std::string& code, ShaderType type);

    /// Constructs a shader from the code provided.
    void operator()(const std::string& code, ShaderType type);
    ~Shader();

    ShaderType shader_type = ShaderType::NONE;

    /// Id of the shader
    unsigned int id;
    AssetType GetAssetType() override { return AssetType::SHADER; }

 private:
    explicit Shader(const Shader&) = default;
};

/// <summary>A shader program.</summary>
///
/// Do note that using this as a direct object is not the preferred usage of the class, and
/// @ref ShaderProgram_t is the preferred way of using it.
///
/// Using @ref ShaderDefinition to load and initialize a shader is also the preferred way of using this class.
class ShaderProgram {
 public:
    ShaderProgram();
    ShaderProgram(const Shader& vert, const Shader& frag);
    ShaderProgram(const Shader& vert, const Shader& frag, const Shader& geom);
    ~ShaderProgram();

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec2(const std::string& name, float x, float y);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec3(const std::string& name, float x, float y, float z);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat);
    void setMat3(const std::string& name, const glm::mat3& mat);
    void setMat4(const std::string& name, const glm::mat4& mat);

    // Simpler overloaded functions so that you can just say set xxx and change the type
    // as and when you like.
    void Set(const std::string& name, bool value);
    void Set(const std::string& name, int value);
    void Set(const std::string& name, float value);
    void Set(const std::string& name, const glm::vec2& value);
    void Set(const std::string& name, float x, float y);
    void Set(const std::string& name, const glm::vec3& value);
    void Set(const std::string& name, float x, float y, float z);
    void Set(const std::string& name, const glm::vec4& value);
    void Set(const std::string& name, float x, float y, float z, float w);
    void Set(const std::string& name, const glm::mat2& mat);
    void Set(const std::string& name, const glm::mat3& mat);
    void Set(const std::string& name, const glm::mat4& mat);

    void UseProgram();
    unsigned int program;

    void SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

    operator unsigned int() const { return program; }
};

/// The preferred way of using a shader program
typedef std::shared_ptr<ShaderProgram> ShaderProgram_t;

/// Helper function to create a shader program
inline ShaderProgram_t MakeShaderProgram(Shader& vert, Shader& frag) {
    return std::make_shared<ShaderProgram>(vert, frag);
}

/// Helper function to create a shader program with a geometry shader
inline ShaderProgram_t MakeShaderProgram(Shader& vert, Shader& frag, Shader& geom) {
    return std::make_shared<ShaderProgram>(vert, frag, geom);
}

/// A shader definiton file.
/// <br />
/// A shader defintion is a way to simplify the creation of shader objects.
/// <br />
/// A shader definiton file is loaded in hjson, and follows this format:
/// ```
/// {
///     vert: (vertex shader name)
///     frag: (fragment shader name)
///     uniforms: {
///         test_vec3: [1, 3, 5]
///         test_float: 19.5
///         test_int: 15
///         test_texture_name: (texture id)
///     }
/// }
/// ```
/// For uniforms, the shader will read the types of uniforms in the shader, and
/// use them.
///
/// Matrices are not supported yet
class ShaderDefinition : public Asset {
 public:
    /// Code for vertex shader
    std::string vert;

    /// Code for fragment shader
    std::string frag;

    /// Code for geometry file
    std::string geometry;

    // All the uniforms
    Hjson::Value uniforms;

    ShaderProgram_t MakeShader();

    AssetType GetAssetType() override { return AssetType::SHADER_DEFINITION; }

 private:
    void SetShaderUniform(cqsp::asset::ShaderProgram_t& shader, std::pair<const std::string, Hjson::Value>& value);
};

// Set of utility functions that load shaders
unsigned int LoadShaderData(const std::string& code, int type);
unsigned int MakeShaderProgram(int vertex, int fragment, int geometry = -1);

/// <summary>
/// Get the issues of the shader, up to 1024 characters of it.
/// </summary>
/// <param name="shader">shader id</param>
/// <returns>the error text of the shader</returns>
std::string GetErrorLog(unsigned int shader);
}  // namespace asset
}  // namespace cqsp
