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
#pragma once

#include <map>
#include <string>

#include "engine/asset/asset.h"
#include "engine/asset/vfs/vfs.h"

namespace cqsp::asset {
struct AssetEntry {
    std::string path;
    AssetType type;
    AssetEntry() : path(""), type(AssetType::NONE) {}
    AssetEntry(const std::string& _path, AssetType _type) : path(_path), type(_type) {}
};

/**
 * A way for the program to quickly load all the files in the 
 */
class PackageIndex {
 public:
    explicit PackageIndex(IVirtualDirectoryPtr directory);

    AssetEntry& operator[](const std::string& name) { return assets[name]; }

 private:
    std::string path;
    bool valid;
    std::map<std::string, AssetEntry> assets;
};
}  // namespace cqsp::asset
