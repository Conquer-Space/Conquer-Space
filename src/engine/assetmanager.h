/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <hjson.h>
#include <imgui.h>

#include <map>
#include <string>
#include <memory>
#include <istream>

#include <boost/lockfree/queue.hpp>

#include "engine/renderer/texture.h"
#include "engine/asset.h"

namespace conquerspace {
namespace asset {

enum class AssetType { NONE, TEXTURE, SHADER, HJSON, TEXT, MODEL };
enum PrototypeType { NONE = 0, TEXTURE, SHADER };

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

class ShaderPrototype : public Prototype {
 public:
    std::string key;
    std::string data;
    int type;

    int GetPrototypeType() {
        return PrototypeType::SHADER;
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

class AssetLoader;

class AssetManager {
 public:
    AssetManager();

    size_t GetSize() { return assets.size(); }
    std::unique_ptr<Asset>& operator[](std::string& key) { return assets[key]; }
    std::unique_ptr<Asset>& operator[](const char* key) { return assets[key]; }

    std::unique_ptr<Asset>& operator[](char* val) { return assets[std::string(val)]; }

    template <class T>
    T* GetAsset(std::string& key) {
#ifndef NDEBUG
        assert(assets.count(key) && "Invalid key!\n");
#endif  // !NDEBUG
        return dynamic_cast<T*>(assets[key].get());
    }

    template <class T>
    T* GetAsset(const char* key) {
#ifndef NDEBUG
        assert(assets.count(key) && "Invalid key!\n");
#endif  // !NDEBUG
        return dynamic_cast<T*>(assets[key].get());
    }

    template <class T>
    T* GetAsset(char* key) {
#ifndef NDEBUG
        assert(assets.count(key) && "Invalid key!\n");
#endif  // !NDEBUG
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

    bool QueueHasItems() { return !m_asset_queue.empty(); }

 private:
     std::unique_ptr<TextAsset> LoadText(std::istream &asset_stream,
                                      Hjson::Value hints);
    std::unique_ptr<HjsonAsset> LoadHjson(std::istream &asset_stream,
                                      Hjson::Value hints);

     void LoadImage(std::string& key, std::string& filePath, Hjson::Value hints);

     void LoadShader(std::string& key, std::istream &asset_stream, Hjson::Value hints);

    std::map<std::string, AssetType> asset_type_map;

    boost::lockfree::queue<QueueHolder> m_asset_queue;
};
}  // namespace asset
}  // namespace conquerspace
