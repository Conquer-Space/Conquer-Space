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

#include <hjson.h>

#include <spdlog/spdlog.h>

#include <map>
#include <string>
#include <memory>
#include <istream>
#include <utility>
#include <vector>

#include <optional>
#include <queue>

#include "engine/renderer/texture.h"
#include "engine/renderer/shader.h"
#include "engine/renderer/text.h"
#include "engine/asset.h"
#include "engine/gui.h"

namespace cqsp {
namespace asset {

enum class AssetType { NONE, TEXTURE, SHADER, HJSON, TEXT, MODEL, FONT, CUBEMAP, TEXT_ARRAY, AUDIO };
enum PrototypeType { NONE = 0, TEXTURE, SHADER, FONT, CUBEMAP };

/**
* Asset Prototypes are for assets that need additional processing
* in the main thread, such as images.
*/
class AssetPrototype {
 public:
    std::string key;
    /// <summary>
    /// Store the asset here so that at least we have the promise of an asset to the thing
    /// </summary>
    Asset* asset;
    virtual int GetPrototypeType() { return PrototypeType::NONE; }
};

/**
* Holds a prototype in the queue to be loaded in the main thread
*/
class QueueHolder {
 public:
    QueueHolder() { prototype = nullptr; }
    explicit QueueHolder(AssetPrototype* type) : prototype(type) {}

    AssetPrototype* prototype;
};

/**
* Queue to hold the asset prototypes.
*/
template <typename T>
class ThreadsafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mutex_;

    // Moved out of public interface to prevent races between this
    // and pop().
    bool empty() const { return queue_.empty(); }

 public:
    ThreadsafeQueue() = default;
    ThreadsafeQueue(const ThreadsafeQueue<T>&) = delete;
    ThreadsafeQueue& operator=(const ThreadsafeQueue<T>&) = delete;

    ThreadsafeQueue(ThreadsafeQueue<T>&& other) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_ = std::move(other.queue_);
    }

    virtual ~ThreadsafeQueue() {}

    unsigned long size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return {};
        }
        T tmp = queue_.front();
        queue_.pop();
        return tmp;
    }

    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }
};

class AssetLoader;
class AssetManager;

class Package {
 public:
    std::string name;
    std::string version;
    std::string title;
    std::string author;

    template<class T, typename V>
    T* GetAsset(const V asset) {
        if (!HasAsset(asset)) {
            SPDLOG_ERROR("Invalid key {}", asset);
        }
        return dynamic_cast<T*>(assets[asset].get());
    }

    bool HasAsset(const char* asset);
    bool HasAsset(const std::string& asset);

 private:
    std::map<std::string, std::unique_ptr<Asset>> assets;

    void ClearAssets();

    friend class AssetLoader;
    friend class AssetManager;
};

class PackagePrototype {
 public:
    std::string name;
    std::string version;
    std::string title;
    std::string author;
    std::string path;

    bool enabled;
};

class AssetManager {
 public:
    AssetManager();

    [[deprecated]]
    std::unique_ptr<Asset>& operator[](std::string& key) {
        // Split the stuff
        int separation = key.find(":");
        std::string token = key.substr(0, separation);
        std::string pkg_key = key.substr(separation, key.length());
        return packages[token]->assets[pkg_key];
    }

    [[deprecated]]
    std::unique_ptr<Asset>& operator[](const char* key) {
        std::string str(key);
        int separation = str.find(":");
        std::string token = str.substr(0, separation);
        std::string pkg_key = str.substr(separation, str.length());
        return packages[token]->assets[pkg_key];
    }

    cqsp::asset::ShaderProgram* CreateShaderProgram(const std::string &vert, const std::string &frag);

    template <class T>
    T* GetAsset(const std::string& key) {
        int separation = key.find(":");
        std::string token = key.substr(0, separation);
        // Check if package exists
        if (packages.count(token) == 0) {
            SPDLOG_ERROR("Cannot find package {}", token);
        }
        std::string pkg_key = key.substr(separation+1, key.length());
        // Probably a better way to do this, to be honest
        // Load default texture
        if (!packages[token]->HasAsset(pkg_key)) {
            SPDLOG_ERROR("Cannot find asset {}", pkg_key);
            if constexpr (std::is_same<T, asset::Texture>::value) {
                return &empty_texture;
            }
        }
        // Check if asset exists
        return packages[token]->GetAsset<T>(pkg_key);
    }

    void LoadDefaultTexture();
    void ClearAssets();

    Package* GetPackage(const std::string& name) {
        return packages[name].get();
    }

    int GetPackageCount() {
        return packages.size();
    }

    auto GetPackageBegin() {
        return packages.begin();
    }

    auto GetPackageEnd() {
        return packages.end();
    }

    void SaveModList();

    std::map<std::string, PackagePrototype> potential_mods;

 private:
    std::map<std::string, std::unique_ptr<Package>> packages;
    asset::Texture empty_texture;
    friend class AssetLoader;
};

class AssetLoader {
  friend class AssetManager;

 public:
    AssetLoader();

    void LoadAssets();

    /// <summary>
    /// Loads a package
    /// </summary>
    /// <param name="package">Path of the package folder</param>
    std::unique_ptr<Package> LoadPackage(std::string package);
    /*
     * The assets that need to be on the opengl. Takes one asset from the queue
     * and processes it
     */
    void BuildNextAsset();

    AssetManager* manager;

    bool QueueHasItems() { return m_asset_queue.size() != 0; }

    std::vector<std::string>& GetMissingAssets() { return missing_assets; }

    /// <summary>
    /// Get where the mod.hjson file is.
    /// </summary>
    /// <returns></returns>
    std::string GetModFilePath();

 private:
    std::string LoadModPrototype(const std::string&);
    void LoadHjsonDirectory(Package& package, std::string path, std::string key);
    std::unique_ptr<TextAsset> LoadText(std::istream &asset_stream,
                                        const Hjson::Value& hints);
    std::unique_ptr<HjsonAsset> LoadHjson(const std::string &path,
                                        const Hjson::Value& hints);
    std::unique_ptr<TextDirectoryAsset> LoadTextDirectory(const std::string& name, const Hjson::Value& hints);

    std::unique_ptr<TextDirectoryAsset> LoadScriptDirectory(const std::string& path, const Hjson::Value& hints);

    void LoadHjson(std::istream &asset_stream, Hjson::Value& value,
                                        const Hjson::Value& hints);
    void LoadHjsonDir(const std::string& path, Hjson::Value& value, const Hjson::Value& hints);
    /// <summary>
    /// Load singular asset
    /// </summary>
    /// <param name="package">package to load into</param>
    /// <param name="type"></param>
    /// <param name="path"></param>
    /// <param name="key"></param>
    /// <param name=""></param>
    void LoadAsset(Package& package, const std::string& type, const std::string& path, const std::string& key,
                    const Hjson::Value&);
    std::unique_ptr<Texture> LoadImage(const std::string& key, const std::string& filePath, const Hjson::Value& hints);


    std::unique_ptr<Shader> LoadShader(const std::string& key, std::istream &asset_stream, const Hjson::Value& hints);
    std::unique_ptr<Font> LoadFont(const std::string& key, std::istream &asset_stream, const Hjson::Value& hints);
    std::unique_ptr<Texture> LoadCubemap(const std::string& key, const std::string &path,
                        std::istream &asset_stream, const Hjson::Value& hints);
    std::map<std::string, AssetType> asset_type_map;
    std::vector<std::string> missing_assets;
    ThreadsafeQueue<QueueHolder> m_asset_queue;
};
}  // namespace asset
}  // namespace cqsp
