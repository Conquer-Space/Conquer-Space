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
#include <memory>
#include <string>

#include "engine/asset/asset.h"
#include "engine/enginelogger.h"

namespace cqsp::asset {
class AssetLoader;
class AssetManager;

class Package {
 public:
    std::string name;
    std::string version;
    std::string title;
    std::string author;

    template <class T, typename V>
    T* GetAsset(const V asset) {
        if (!HasAsset(asset)) {
            ENGINE_LOG_ERROR("Invalid key {}", asset);
            return nullptr;
        }
        return dynamic_cast<T*>(assets[asset].get());
    }

    bool HasAsset(const char* asset);
    bool HasAsset(const std::string& asset);

    auto begin() { return assets.begin(); }

    auto end() { return assets.end(); }

 private:
    std::map<std::string, std::unique_ptr<Asset>> assets;

    void ClearAssets();

    friend class AssetLoader;
    friend class AssetManager;
};

}  // namespace cqsp::asset
