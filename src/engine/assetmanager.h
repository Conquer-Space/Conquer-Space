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
#include <vector>

#include <optional>
#include <queue>

#include "engine/renderer/texture.h"
#include "engine/renderer/shader.h"
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

class AssetManager {
 public:
    AssetManager();

    size_t GetSize() { return assets.size(); }
    std::unique_ptr<Asset>& operator[](std::string& key) { return assets[key]; }
    std::unique_ptr<Asset>& operator[](const char* key) { return assets[key]; }

    std::unique_ptr<Asset>& operator[](char* val) { return assets[std::string(val)]; }

    cqsp::asset::ShaderProgram* CreateShaderProgram(const std::string &vert,
                                                            const std::string &frag);

    template <class T, typename K>
    T* GetAsset(const K key) {
        if (!assets.count(key)) {
            SPDLOG_ERROR("Invalid key {}", key);
            if constexpr (std::is_same<T, asset::Texture>::value) {
                // Then load default texture
                return &empty_texture;
            }
        }
        return dynamic_cast<T*>(assets[key].get());
    }

    void LoadDefaultTexture();
    void ClearAssets();

 private:
    template<class T>
    void AddAsset(std::string key, std::unique_ptr<T> &&asset) {
        // Check for asset collision
        if (assets.find(key) != assets.end()) {
            SPDLOG_WARN("Didn't add asset {} due to asset collision", key);
        } else {
            assets[key] = std::move(asset);
        }
    }
    std::map<std::string, std::unique_ptr<Asset>> assets;
    asset::Texture empty_texture;
    friend class AssetLoader;
};

class AssetLoader {
  friend class AssetManager;

 public:
    AssetLoader();

    void LoadAssets(std::istream&);

    /*
     * The assets that need to be on the opengl. Takes one asset from the queue
     * and processes it
     */
    void BuildNextAsset();

    AssetManager* manager;

    bool QueueHasItems() { return m_asset_queue.size() != 0; }

    std::vector<std::string>& GetMissingAssets() { return missing_assets; }

 private:
    std::unique_ptr<TextAsset> LoadText(std::istream &asset_stream,
                                        const Hjson::Value& hints);
    std::unique_ptr<HjsonAsset> LoadHjson(const std::string &path,
                                        const Hjson::Value& hints);
    std::unique_ptr<TextDirectoryAsset> LoadTextDirectory(const std::string& name, const Hjson::Value& hints);
    void LoadHjson(std::istream &asset_stream, Hjson::Value& value,
                                        const Hjson::Value& hints);
    void LoadHjsonDir(const std::string& path, Hjson::Value& value, const Hjson::Value& hints);
    // Load singular asset
    void LoadAsset(const std::string&, const std::string&, const std::string&, const Hjson::Value&);
    void LoadImage(const std::string& key, const std::string& filePath, const Hjson::Value& hints);

    void LoadShader(const std::string& key, std::istream &asset_stream, const Hjson::Value& hints);
    void LoadFont(const std::string& key, std::istream &asset_stream, const Hjson::Value& hints);
    void LoadCubemap(const std::string& key, const std::string &path,
                        std::istream &asset_stream, const Hjson::Value& hints);
    std::map<std::string, AssetType> asset_type_map;

    std::vector<std::string> missing_assets;
    ThreadsafeQueue<QueueHolder> m_asset_queue;
};
}  // namespace asset
}  // namespace cqsp
