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

#include <string>

namespace cqsp {
namespace asset {
enum class AssetType {
    NONE, TEXTURE, SHADER, HJSON, TEXT, MODEL, FONT, CUBEMAP, TEXT_ARRAY, AUDIO
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
    }
    else if (name == "texture") {
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
    }
}

class Asset {
 public:
    // Virtual destructor to make class virtual
    virtual ~Asset(){}

    std::string original_path;

    virtual AssetType GetAssetType() { return AssetType::NONE; }
};
}  // namespace asset
}  // namespace cqsp
