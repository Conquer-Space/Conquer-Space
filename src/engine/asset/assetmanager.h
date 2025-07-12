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

#include <hjson.h>
#include <spdlog/spdlog.h>

#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "engine/asset/asset.h"
#include "engine/asset/package.h"
#include "engine/asset/packageprototype.h"
#include "engine/asset/textasset.h"
#include "engine/asset/vfs/vfs.h"
#include "engine/engine.h"
#include "engine/enginelogger.h"
#include "engine/graphics/shader.h"
#include "engine/graphics/text.h"
#include "engine/graphics/texture.h"
#include "engine/gui.h"
#include "engine/util/threadsafequeue.h"

namespace cqsp::asset {
class AssetLoader;
class AssetManager {
 public:
    AssetManager() = default;

    ShaderProgram_t MakeShader(const std::string& vert, const std::string& frag);
    ShaderProgram_t MakeShader(const std::string& vert, const std::string& frag, const std::string& geom);

    /// <summary>
    /// Gets an asset.
    /// </summary>
    /// To get an asset, it defaults finding the asset in `core` if you do not specify a package,
    /// or else if the asset is from another asset pack, you can specify
    /// `mod_name:asset_name`, the separator between the two being a colon.
    /// <typeparam name="T">The type class</typeparam>
    /// <param name="key"></param>
    /// <returns></returns>
    template <class T>
    T* GetAsset(const std::string& key) {
        static_assert(std::is_base_of<Asset, T>::value, "Class is not child of cqsp::asset::Asset");
        std::size_t separation = key.find(":");
        // Default name is core
        std::string package_name = "core";
        if (separation != std::string::npos) {
            package_name = key.substr(0, separation);
        }

        // Check if package exists
        if (packages.count(package_name) == 0) {
            ENGINE_LOG_ERROR("Cannot find package {}", package_name);
            return nullptr;
        }
        std::string pkg_key = key.substr(separation + 1, key.length());
        auto& package = packages[package_name];
        // Probably a better way to do this, to be honest
        // Load default texture
        if (!package->HasAsset(pkg_key)) {
            ENGINE_LOG_ERROR("Cannot find asset {}", pkg_key);
            if constexpr (std::is_same<T, asset::Texture>::value) {
                return &empty_texture;
            }
            return nullptr;
        }
        // Check if asset exists
        T* ptr = package->GetAsset<T>(pkg_key);
        if (ptr == nullptr) {
            SPDLOG_WARN("Asset {} is wrong type", key);
        } else {
            ptr->accessed++;
        }
        ptr->PostLoad(*this);
        return ptr;
    }

    void LoadDefaultTexture();
    void ClearAssets();

    Package* GetPackage(const std::string& name) { return packages[name].get(); }

    size_t GetPackageCount() { return packages.size(); }

    auto begin() { return packages.begin(); }

    auto end() { return packages.end(); }

    void SaveModList();

    std::map<std::string, PackagePrototype> m_package_prototype_list;

 private:
    std::map<std::string, std::unique_ptr<Package>> packages;
    asset::Texture empty_texture;
    friend class AssetLoader;
};
}  // namespace cqsp::asset
