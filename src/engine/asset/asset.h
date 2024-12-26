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

#include <string>

/// <summary>
/// When adding assets, it is extremely crucial that you read
/// @ref cqsp::asset::AssetLoader::LoadResources to find out how to
/// add a resource to the game.
///
/// When loading an asset from the game, read
/// @ref cqsp::asset::AssetLoader::GetAsset
/// </summary>
namespace cqsp {
namespace asset {
class AssetManager;

enum class AssetType {
    NONE,
    /// \see @ref cqsp::asset::AssetLoader::LoadTexture for implementation
    /// <summary>
    /// `texture` as the `type` parameter in `resource.hjson`
    /// </summary>
    TEXTURE,
    /// \see @ref cqsp::asset::AssetLoader::LoadShader for implementation
    /// <summary>
    /// `shader` as the `type` parameter in `resource.hjson`
    /// </summary>
    SHADER,
    /// \see @ref cqsp::asset::AssetLoader::LoadHjson for implementation
    /// <summary>
    /// `hjson` as the `type` parameter in `resource.hjson`
    /// </summary>
    HJSON,
    /// <summary>
    /// `text` as the `type` parameter in `resource.hjson`
    /// </summary>
    /// \see @ref cqsp::asset::AssetLoader::LoadText for implementation
    TEXT,
    /// <summary>
    /// Not supported yet.
    /// </summary>
    MODEL,
    /// <summary>
    /// `cubemap` as the `type` parameter in `resource.hjson`
    /// </summary>
    /// \see @ref cqsp::asset::AssetLoader::LoadFont for implementation
    FONT,
    /// <summary>
    /// `cubemap` as the `type` parameter in `resource.hjson`
    /// </summary>
    /// \see @ref cqsp::asset::AssetLoader::LoadCubemap for implementation
    CUBEMAP,
    /// \see @ref cqsp::asset::AssetLoader::LoadTextDirectory for implementation
    TEXT_ARRAY,
    /// <summary>
    /// `audio` as the `type` parameter in `resource.hjson`
    /// </summary>
    /// \see @ref cqsp::asset::AssetLoader::LoadAudio for implementation
    AUDIO,
    /// <summary>
    /// `shader_def` as the `type` parameter in `resource.hjson`
    /// </summary>
    /// \see @ref cqsp::asset::AssetLoader::LoadShaderDefinition for implementation
    SHADER_DEFINITION,
    SCRIPT,
    BINARY
};

// This will be hell to maintain

/// <summary>
/// Converts asset type names to asset types
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
inline AssetType FromString(const std::string& name) {
    if (name == "none") {
        return AssetType::NONE;
    } else if (name == "texture") {
        return AssetType::TEXTURE;
    } else if (name == "shader") {
        return AssetType::SHADER;
    } else if (name == "hjson") {
        return AssetType::HJSON;
    } else if (name == "text") {
        return AssetType::TEXT;
    } else if (name == "model") {
        return AssetType::MODEL;
    } else if (name == "font") {
        return AssetType::FONT;
    } else if (name == "cubemap") {
        return AssetType::CUBEMAP;
    } else if (name == "directory") {
        return AssetType::TEXT_ARRAY;
    } else if (name == "audio") {
        return AssetType::AUDIO;
    } else if (name == "shader_def") {
        return AssetType::SHADER_DEFINITION;
    } else if (name == "binary") {
        return AssetType::BINARY;
    }
    return AssetType::NONE;
}

/// <summary>
/// Converts asset type to asset type names for display
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
inline std::string ToString(AssetType type) {
    switch (type) {
        default:
        case AssetType::NONE:
            return "none";
        case AssetType::TEXTURE:
            return "texture";
        case AssetType::SHADER:
            return "shader";
        case AssetType::HJSON:
            return "hjson";
        case AssetType::TEXT:
            return "text";
        case AssetType::MODEL:
            return "model";
        case AssetType::FONT:
            return "font";
        case AssetType::CUBEMAP:
            return "cubemap";
        case AssetType::TEXT_ARRAY:
            return "directory";
        case AssetType::AUDIO:
            return "audio";
        case AssetType::BINARY:
            return "binary";
        case AssetType::SHADER_DEFINITION:
            return "shader_def";
    }
}

/// <summary>
/// The base class for assets.
/// </summary>
class Asset {
 public:
    // Virtual destructor to make class virtual
    virtual ~Asset() = default;
    virtual AssetType GetAssetType() = 0;
    // For lazy initialization or other
    virtual void PostLoad(AssetManager&) {}
    std::string path;
    int accessed = 0;
};
}  // namespace asset
}  // namespace cqsp
