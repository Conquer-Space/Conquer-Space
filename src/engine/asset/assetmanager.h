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

#include "engine/engine.h"
#include "engine/asset/asset.h"
#include "engine/asset/textasset.h"
#include "engine/graphics/texture.h"
#include "engine/graphics/shader.h"
#include "engine/graphics/text.h"
#include "engine/gui.h"
#include "engine/asset/vfs/vfs.h"

namespace cqsp {
namespace asset {

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

    ShaderProgram_t MakeShader(const std::string &vert, const std::string &frag);

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
 public:
    AssetLoader();

    /// <summary>
    /// Checks for all the loaded mods, and loads the assets that the mod loads.
    ///
    /// It loads first, then loads the mod metadata, then it loads the mods that it needs to load.
    /// </summary>
    void LoadAssets();

    /// <summary>
    /// Loads a package.
    /// <br>
    /// A package contains a few things that it loads by default.
    /// First it loads the info.hjson file to check for all the metadata about the package.
    /// The metadata of the package looks like this:
    /// ```
    /// {
    ///     name: # Namespace name of the package. Since packages are loaded with a colon, please refrain from
    ///           # adding colons in the package name
    ///     version: # Version of the package
    ///     title: # What you want to call the package
    ///     author: # Who wrote the package
    ///     # Other information can go here, like contacts and other things, but we won't care about it so far
    ///     dependencies: []
    /// }
    /// ```
    /// After that, it will load a couple of important folders.
    /// <br>
    /// The base script file, `scripts/base.lua`, will be loaded into the asset name `base` as a text file.
    /// Next it loads the script folder, `scripts/` into a `TextDirectoryAsset` pointer, to the asset `scripts`
    /// <br>
    /// Then, it loads the goods and recipes as hjson, with the names `goods` and `recipes` respectively.
    /// <br>
    /// Finally, it recursively loads all the `resource.hjson` files in the folders. It is not reccomended to override
    /// the preloaded resources, but it is possible. If you really want to do so, it is strongly reccomended to keep
    /// type of asset the same for the key.
    /// </summary>
    /// <param name="package">Path to package folder</param>
    /// <returns>The uniqueptr to the package that is created</returns>
    std::unique_ptr<Package> LoadPackage(std::string package);

    /// <summary>
    /// The assets that need to be on the main thread. Takes one asset from the queue and processes it
    /// </summary>
    void BuildNextAsset();


    /// <summary>
    /// Checks if the queue has any remaining items to load on the main thread or not.
    /// </summary>
    /// <returns></returns>
    bool QueueHasItems() { return m_asset_queue.size() != 0; }

    /// <summary>
    /// Gets the list of assets that were listed in the resource.hjson files, but were not found.
    /// </summary>
    /// <returns></returns>
    std::vector<std::string>& GetMissingAssets() { return missing_assets; }

    /// <summary>
    /// Get where the mod.hjson file is.
    /// </summary>
    /// <returns></returns>
    std::string GetModFilePath();

    std::atomic_int& getMaxLoading() { return max_loading; }
    std::atomic_int& getCurrentLoading() { return currentloading; }

    friend class AssetManager;

    AssetManager* manager;

    typedef std::function<std::unique_ptr<Asset>
        (cqsp::asset::VirtualMounter*, const std::string&, const std::string&, const Hjson::Value&)> LoaderFunction;
 private:
    std::string LoadModPrototype(const std::string&);

    std::unique_ptr<cqsp::asset::Asset> LoadText(
        cqsp::asset::VirtualMounter* f, const std::string& path, const std::string& key,
        const Hjson::Value& hints);
    std::unique_ptr<cqsp::asset::Asset> LoadTextDirectory(
        cqsp::asset::VirtualMounter* f, const std::string& path, const std::string& key,
        const Hjson::Value& hints);
    std::unique_ptr<cqsp::asset::Asset> LoadTexture(
        cqsp::asset::VirtualMounter* f, const std::string& path,
        const std::string& key, const Hjson::Value& hints);
    std::unique_ptr<cqsp::asset::Asset> LoadHjson(
        cqsp::asset::VirtualMounter* f, const std::string& path,
        const std::string& key, const Hjson::Value& hints);

    std::unique_ptr<TextAsset> LoadText(std::istream &asset_stream, const Hjson::Value& hints);
    std::unique_ptr<TextDirectoryAsset> LoadTextDirectory(const std::string& name, const Hjson::Value& hints);
    /// <summary>
    /// Loads a script directory.
    /// </summary>
    /// <param name="path"></param>
    /// <param name="hints"></param>
    /// <returns></returns>
    std::unique_ptr<TextDirectoryAsset> LoadScriptDirectory(const std::string& path, const Hjson::Value& hints);

    /// <summary>
    /// Loads a hjson asset that is assumed to be hjson array into the `value` parameter.
    /// </summary>
    /// <param name="path">The path of the hjson file</param>
    /// <param name="value">The hjson array where the data in the asset will be placed into</param>
    /// <param name="hints">A formality, is not used.</param>
    void LoadHjsonFromArray(std::istream &asset_stream, Hjson::Value& value, const Hjson::Value& hints);

    /// <summary>
    /// Loads all the hjson in a directory into one hjson object. All the hjson has to be in an array.
    /// </summary>
    /// <param name="path"></param>
    /// <param name="value"></param>
    /// <param name="hints"></param>
    void LoadHjsonDir(const std::string& path, Hjson::Value& value, const Hjson::Value& hints);

    /// <summary>
    /// Loads hjson asset from file. Does not matter if it's an array or an object.
    /// </summary>
    /// <param name="path"></param>
    /// <param name="hints"></param>
    /// <returns></returns>
    std::unique_ptr<HjsonAsset> LoadHjson(const std::string &path, const Hjson::Value& hints);

    /// <summary>
    /// Load hjson asset from file into a package, and all the hjson needs to be arrays.
    /// </summary>
    /// <param name="package"></param>
    /// <param name="path"></param>
    /// <param name="key"></param>
    void LoadHjsonDirectory(Package& package, std::string path, std::string key);

    /// <summary>
    /// Load singular asset
    /// </summary>
    /// <param name="package">package to load into</param>
    /// <param name="type">Type of asset the asset loader says it shoul dload</param>
    /// <param name="path">The real path where the asset is lcated</param>
    /// <param name="key">Key name of the asset</param>
    /// <param name="hints">Any extra information for the asset loader to take into account</param>
    void LoadAsset(Package& package, const std::string& type, const std::string& path, const std::string& key,
                    const Hjson::Value& hints);
    void LoadAsset(Package& package, const AssetType& type,
                   const std::string& path, const std::string& key,
                   const Hjson::Value& hints);

    std::unique_ptr<Texture> LoadTexture(const std::string& key, const std::string& filePath,
                    const Hjson::Value& hints);
    std::unique_ptr<Texture> LoadCubemap(const std::string& key, const std::string &path,
                    std::istream &asset_stream, const Hjson::Value& hints);

    std::unique_ptr<Shader> LoadShader(const std::string& key, std::istream &asset_stream, const Hjson::Value& hints);
    std::unique_ptr<Font> LoadFont(const std::string& key, std::istream &asset_stream, const Hjson::Value& hints);

    /// <summary>
    /// Loads any type of directory, and executes the function for every single file.
    /// <br>
    /// This is a helper function to add to calculate the number of assets loaded so far.
    /// </summary>
    /// <param name="path">Path of directory to read</param>
    /// <param name="file">Function pointer to do something with the path, and it takes the path of the
    /// asset as the parameter</param>
    void LoadDirectory(std::string path, std::function<void(std::string)> file);

    /// <summary>
    /// Loads all the `resource.hjson` files in the specified directory.
    /// <br>
    /// `resource.hjson` files consist of a hjson object.
    /// This is the syntax:
    /// ```
    /// {
    ///     test_asset: { # Asset name
    ///         path: # Path relative to the current resource.hjson file.
    ///         type: # Type of asset: &lt;none|texture|shader|hjson|text|model|font|cubemap|directory|audio&rt;
    ///         hints: {} # Object of whatever information you want when loading the asset.
    ///     }
    /// }
    /// ```
    /// <br>
    /// The asset manager allows for a relatively wide variety of assets.
    /// <br>
    /// ### Images
    /// Images are gonna be one of the most common things loaded from the asset manager.
    /// Textures have one hint, the `magfilter` hint. If it is present, and set to true, it will enable
    /// closest magfilter, which will make the texture look pixellated.
    /// If it is not present, then it will be linear mag.
    /// <br>
    /// ### Cubemaps
    /// Cubemaps are a special type of texture that make up the skybox. They consist of 6 textures.
    /// But, when loading it, you have to specify a hjson file that links the files of the asset.
    /// The syntax of the hjson file will be a simple array of strings, specifing the relative paths
    /// relative to that hjson file. You will have 6 images, specifying the left, right, top, bottom,
    /// back, and front images respectively in that order.
    /// <br>
    /// ### Fonts
    /// Just specify a .ttf file, and it will load it. Pretty simple.
    /// <br>
    /// ### Shader
    /// Shaders have one option, the `type` hint, to specify what type of shader it is.
    /// We have two so far, the `frag` option for a fragment shader, and `vert` for a vertex shader.
    /// We do not have support for compute and geometry shaders, but we may add support for that in the future.
    /// <br>
    /// ### Hjson
    /// Hjson is rather flexable, it can load a single file or a directory, with just the same option.
    /// If the input path is a file, then it will load the hjson file into a `Hjson::Value`.
    /// If the input path is a directory, then it will load all the hjson files in the directory into a hjson value.
    /// This is for large amounts of data that would be better to be split up across files. The data in each file is
    /// assumed to be an array of objects, and so will load all the arrays of hjson objects in each of the hjson
    /// files into one hjson object.
    /// <br>
    /// ### Text
    /// Pretty straightforward, loads a text file.
    /// <br>
    /// The following assets do nothing, either because we don't have support for them yet, or some other
    /// reason:
    /// model, none, directory
    /// <br>
    /// We don't have model support yet, but that will come soon.
    /// </summary>
    /// <param name="path">Base virtual path to the package</param>
    void LoadResources(Package& package, std::string path);

    /// <summary>
    /// Creates a virtual file system for the path to load.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    IVirtualFileSystem* GetVfs(const std::string& path);

    std::vector<std::string> missing_assets;
    ThreadsafeQueue<QueueHolder> m_asset_queue;

    std::atomic_int max_loading;
    std::atomic_int currentloading;
    std::map<AssetType, LoaderFunction> loading_functions;
    VirtualMounter mounter;
};
}  // namespace asset
}  // namespace cqsp
