/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <hjson.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

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

namespace conquerspace {
namespace asset {

enum class AssetType { NONE, TEXTURE, SHADER, HJSON, TEXT, MODEL, FONT, CUBEMAP, TEXT_ARRAY };
enum PrototypeType { NONE = 0, TEXTURE, SHADER, FONT, CUBEMAP };

class Prototype {
 public:
    virtual int GetPrototypeType() { return PrototypeType::NONE; }
};

class ImagePrototype : public Prototype{
 public:
    char* key;
    unsigned char* data;
    int width;
    int height;
    int components;

    asset::TextureLoadingOptions options;

    int GetPrototypeType() { return PrototypeType::TEXTURE; }
};

class CubemapPrototype : public Prototype{
 public:
    char* key;
    std::vector<unsigned char*> data;
    int width;
    int height;
    int components;

    asset::TextureLoadingOptions options;

    int GetPrototypeType() { return PrototypeType::CUBEMAP; }
};

class ShaderPrototype : public Prototype {
 public:
    std::string key;
    std::string data;
    int type;

    int GetPrototypeType() {
        return PrototypeType::SHADER;
    }
};

class FontPrototype : public Prototype {
 public:
    unsigned char* fontBuffer;
    int size;
    std::string key;

    int GetPrototypeType() {
        return PrototypeType::FONT;
    }
};

/*
* Holds a prototype in the queue.
*/
class QueueHolder {
 public:
    QueueHolder() { prototype = nullptr; }
    explicit QueueHolder(Prototype* type) : prototype(type) {}

    Prototype* prototype;
};

// Queue
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

    conquerspace::asset::ShaderProgram* CreateShaderProgram(const std::string &vert,
                                                            const std::string &frag);

    template <class T>
    T* GetAsset(std::string& key) {
        if (!assets.count(key))
            SPDLOG_ERROR("Invalid key {}", key);
        return dynamic_cast<T*>(assets[key].get());
    }

    template <class T>
    T* GetAsset(const char* key) {
        if (!assets.count(key))
            SPDLOG_ERROR("Invalid key {}", key);
        return dynamic_cast<T*>(assets[key].get());
    }

    template <class T>
    T* GetAsset(char* key) {
        if (!assets.count(key))
            SPDLOG_ERROR("Invalid key {}", key);
        return dynamic_cast<T*>(assets[key].get());
    }

 private:
    std::map<std::string, std::unique_ptr<Asset>> assets;

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

 private:
    std::unique_ptr<TextAsset> LoadText(std::istream &asset_stream,
                                      Hjson::Value& hints);
    std::unique_ptr<HjsonAsset> LoadHjson(std::istream &asset_stream,
                                      Hjson::Value& hints);
    std::unique_ptr<TextDirectoryAsset> LoadTextDirectory(std::string name, Hjson::Value& hints);
    void LoadHjson(std::istream &asset_stream, Hjson::Value& value,
                                      Hjson::Value& hints);
    void LoadHjsonDir(std::string& path, Hjson::Value& value, Hjson::Value& hints);
    // Load singular asset
    void LoadAsset(std::string&, std::string&, std::string&, Hjson::Value&);
    void LoadImage(std::string& key, std::string& filePath, Hjson::Value& hints);

    void LoadShader(std::string& key, std::istream &asset_stream, Hjson::Value& hints);
    void LoadFont(std::string& key, std::istream &asset_stream, Hjson::Value& hints);
    void LoadCubemap(std::string& key, std::string &path,
                        std::istream &asset_stream, Hjson::Value& hints);

    std::map<std::string, AssetType> asset_type_map;

    ThreadsafeQueue<QueueHolder> m_asset_queue;
};
}  // namespace asset
}  // namespace conquerspace
