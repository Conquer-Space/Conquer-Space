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

#include <map>
#include <string>
#include <vector>

#include "engine/asset/asset.h"
namespace cqsp::asset {
class TextAsset : public Asset {
 public:
    std::string data;
};

/// <summary>
/// Text asset which remembers what it's path is.
/// </summary>
class PathedTextAsset {
 public:
    // Relative path for the asset compared to the resource.hjson
    std::string data;
    std::string path;

    operator std::string() { return data; }
};

/// <summary>
/// Asset to store a directory of text files, and remembers the path of the assets.
/// </summary>
class TextDirectoryAsset : public Asset {
 public:
    // Get the path of the assets
    std::map<std::string, PathedTextAsset> paths;
};

class HjsonAsset : public Asset {
 public:
    Hjson::Value data;
};

class BinaryAsset : public Asset {
 public:
    std::vector<uint8_t> data;
};
}  // namespace cqsp::asset
