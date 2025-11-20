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
#include "engine/asset/assetmanager.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include <assimp/Importer.hpp>
#include <tracy/Tracy.hpp>

#include "core/util/paths.h"
#include "engine/asset/assetprototypedefs.h"
#include "engine/asset/modelloader.h"
#include "engine/asset/package.h"
#include "engine/asset/vfs/nativevfs.h"
#include "engine/audio/alaudioasset.h"
#include "engine/enginelogger.h"
#include "engine/graphics/model.h"

// Definition for prototypes
namespace cqsp::asset {
ShaderProgram_t AssetManager::MakeShader(const std::string& vert, const std::string& frag) {
    return std::make_shared<ShaderProgram>(*GetAsset<Shader>(vert), *GetAsset<Shader>(frag));
}

ShaderProgram_t AssetManager::MakeShader(const std::string& vert, const std::string& frag, const std::string& geom) {
    return std::make_shared<ShaderProgram>(*GetAsset<Shader>(vert), *GetAsset<Shader>(frag), *GetAsset<Shader>(geom));
}

std::string GetShaderCode(const std::string& identifier) { return std::string(); }

void MakeShader(const Hjson::Value& hjson) {
    // Load the shader values
    std::string vert = hjson["vert"].to_string();
    std::string frag = hjson["frag"].to_string();
    // Get the shader code somehow, but idk how

    Shader vert_shader(GetShaderCode(vert), ShaderType::VERT);
    Shader frag_shader(GetShaderCode(frag), ShaderType::FRAG);
    // Create the shader
    ShaderProgram_t shader = MakeShaderProgram(vert_shader, frag_shader);
    // Initial values
    Hjson::Value initial = hjson["initial"];
    for (auto value : initial) {
        switch (value.second.type()) {
            case Hjson::Type::Int64:
            case Hjson::Type::Double: {
                shader->Set(value.first, (float)value.second.to_double());
                break;
            }
            case Hjson::Type::Vector: {
                // Check if matrix or vector
                switch (value.second.size()) {
                    case 2:
                        shader->Set(value.first, (float)value.second[0].to_double(),
                                    (float)value.second[1].to_double());
                        break;
                    case 3:
                        shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double(),
                                    (float)value.second[2].to_double());
                        break;
                    case 4:
                        shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double(),
                                    (float)value.second[2].to_double(), (float)value.second[3].to_double());
                        break;
                    default:
                        // Not the right size, ignore!
                        break;
                }
                break;
            }

            case Hjson::Type::Undefined:
            case Hjson::Type::Bool:
            case Hjson::Type::Null:
            case Hjson::Type::Map:
            case Hjson::Type::String:
                break;
        }
    }
}

void AssetManager::LoadDefaultTexture() {
    unsigned char texture_bytes[] = {
        0,   0, 0,   255, 0, 255,
        0,   0,  // These two padding bytes are needed for some reason. Opengl doesn't like 2x2 images
        255, 0, 255, 0,   0, 0};

    TextureLoadingOptions f;
    f.mag_filter = true;
    CreateTexture(empty_texture, texture_bytes, 2, 2, 3, f);
}

void AssetManager::ClearAssets() { packages.clear(); }

void AssetManager::SaveModList() {
    Hjson::Value enabled_mods;
    // Load the enabled mods, and write to the file. then exit game.
    for (auto it = m_package_prototype_list.begin(); it != m_package_prototype_list.end(); it++) {
        enabled_mods[it->second.name] = it->second.enabled;
    }
    // Write to file
    std::string mods_path = (std::filesystem::path(core::util::GetCqspAppDataPath()) / "mod.hjson").string();
    Hjson::MarshalToFile(enabled_mods, mods_path);
    ENGINE_LOG_INFO("Writing mods");
}
}  // namespace cqsp::asset
