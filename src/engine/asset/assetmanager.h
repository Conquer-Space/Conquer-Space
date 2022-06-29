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
#include "engine/enginelogger.h"

namespace cqsp {
namespace asset {
enum PrototypeType {
    NONE = 0,
    TEXTURE, //!< texture prototype
    SHADER, //!< shader prototype
    FONT, //!< Font prototype
    CUBEMAP //!< cubemap prototype
};

///
/// Asset Prototypes are for assets that need additional processing
/// in the main thread, such as images.
///
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
            ENGINE_LOG_ERROR("Invalid key {}", asset);
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

    ShaderProgram_t MakeShader(const std::string &vert, const std::string &frag);
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
        static_assert(std::is_base_of<Asset, T>::value,
                      "Class is not child of cqsp::asset::Asset");
        std::size_t separation = key.find(":");
        // Default name is core
        std::string package_name = "core";
        if (separation != std::string::npos) {
            package_name = key.substr(0, separation);
        }

        // Check if package exists
        if (packages.count(package_name) == 0) {
            ENGINE_LOG_ERROR("Cannot find package {}", package_name);
        }
        std::string pkg_key = key.substr(separation+1, key.length());
        auto& package = packages[package_name];
        // Probably a better way to do this, to be honest
        // Load default texture
        if (!package->HasAsset(pkg_key)) {
            ENGINE_LOG_ERROR("Cannot find asset {}", pkg_key);
            if constexpr (std::is_same<T, asset::Texture>::value) {
                return &empty_texture;
            }
        }
        // Check if asset exists
        return package->GetAsset<T>(pkg_key);
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

    std::map<std::string, PackagePrototype> m_package_prototype_list;

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
    void LoadMods();

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
    static std::string GetModFilePath();

    std::atomic_int& getMaxLoading() { return max_loading; }
    std::atomic_int& getCurrentLoading() { return currentloading; }

    friend class AssetManager;

    AssetManager* manager;

    typedef std::function<std::unique_ptr<Asset>
        (cqsp::asset::VirtualMounter* mount,
            const std::string& path, const std::string& key, const Hjson::Value& hints)> LoaderFunction;

 private:
    std::optional<PackagePrototype> LoadModPrototype(const std::string&);

    /// <summary>
    /// Pretty straightforward, loads a text file into a string.
    ///
    /// This has no hints
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadText(cqsp::asset::VirtualMounter* mount,
                                                const std::string& path,
                                                const std::string& key,
                                                const Hjson::Value& hints);
    /// <summary>
    /// Loads a directory of text files into a map of strings keyed by their relative path to the
    /// resource.hjson file.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadTextDirectory(cqsp::asset::VirtualMounter* mount,
                                                         const std::string& path,
                                                         const std::string& key,
                                                         const Hjson::Value& hints);

    /// <summary>
    /// Textures have one hint, the `magfilter` hint. If it is present, and set to true, it will enable
    /// closest magfilter, which will make the texture look pixellated.
    /// If it is not present, then it will be linear mag.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadTexture(cqsp::asset::VirtualMounter* mount,
                                                    const std::string& path,
                                                    const std::string& key,
                                                    const Hjson::Value& hints);

    /// <summary>
    /// Loads binary data straight from the file.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadBinaryAsset(cqsp::asset::VirtualMounter* mount,
                                                        const std::string& path,
                                                        const std::string& key,
                                                        const Hjson::Value& hints);

    /// <summary>
    /// Hjson is rather flexible, it can load a single file or a directory, with just the same option.
    /// If the input path is a file, then it will load the hjson file into a `Hjson::Value`.
    /// If the input path is a directory, then it will load all the hjson files in the directory into a hjson value.
    /// This is for large amounts of data that would be better to be split up across files. The data in each file is
    /// assumed to be an array of objects, and so will load all the arrays of hjson objects in each of the hjson
    /// files into one hjson object.
    ///
    /// If it refers to directory, and a file that is loaded is not in a hjson array, it will not load that specific
    /// file, but it will not fail.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadHjson(cqsp::asset::VirtualMounter* mount,
                                                 const std::string& path,
                                                 const std::string& key,
                                                 const Hjson::Value& hints);

    /// <summary>
    /// Shaders have one option, the `type` hint, to specify what type of shader it is.
    /// We have two so far, the `frag` option for a fragment shader, and `vert` for a vertex shader.
    /// We do not have support for compute and geometry shaders, but we may add support for that in the future.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadShader(cqsp::asset::VirtualMounter* mount,
                                                  const std::string& path,
                                                  const std::string& key,
                                                  const Hjson::Value& hints);
    /// <summary>
    /// Just specify a .ttf file, and it will load it into an opengl texture font object.
    /// </summary>
    /// This is for opengl rendering.
    ///
    /// Because each UI library has their differing implementation of fonts, to add
    /// fonts to the UI, do not use a `resource.hjson`. You need to go to `binaries/data/core/gfx/fonts.hjson`
    /// to alter and add fonts for the UI.
    std::unique_ptr<cqsp::asset::Asset> LoadFont(cqsp::asset::VirtualMounter* mount,
                                                 const std::string& path,
                                                 const std::string& key,
                                                 const Hjson::Value& hints);
    /// <summary>
    /// Only ogg files are supported for now.
    /// </summary>
    /// If you're looking to add a music file, do not add it by the 'normal' way. This is for performance
    /// reasons.
    /// Go to `binaries/data/core/music/readme.txt` for further reading.
    std::unique_ptr<cqsp::asset::Asset> LoadAudio(cqsp::asset::VirtualMounter* mount,
                                                 const std::string& path,
                                                 const std::string& key,
                                                 const Hjson::Value& hints);

    /// <summary>
    /// Cubemaps are a special type of texture that make up the skybox. Although they are specified differently in the
    /// `resource.hjson`, they become textures when they are loaded.
    ///
    /// They consist of 6 textures.
    /// When loading it, you have to specify a hjson file that links the files of the asset.
    /// The syntax of the hjson file will be a simple array of strings, specifing the relative paths
    /// relative to that hjson file. You will have 6 images, specifying the left, right, top, bottom,
    /// back, and front images respectively in that order.
    ///
    /// Example:
    /// ```
    /// [
    ///  left.png // The top left texture
    ///  right.png // You get the gist
    ///  top.png
    ///  bottom.png
    ///  back.png
    ///  front.png // In total there will have 6 textures.
    /// ]
    ///```
    /// This will not load the texture more or less than 6 textures are defined in the array.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadCubemap(cqsp::asset::VirtualMounter* mount,
                                                    const std::string& path,
                                                    const std::string& key,
                                                    const Hjson::Value& hints);

    /// <summary>
    /// A shader definiton file.
    /// <br />
    /// A shader defintion is a way to simplify the creation of shader objects.
    /// <br />
    /// A shader definiton file is loaded in hjson, and follows this format:
    /// ```
    /// {
    ///     vert: (vertex shader name)
    ///     frag: (fragment shader name)
    ///     uniforms: {
    ///         test_vec3: [1, 3, 5]
    ///         test_float: 19.5
    ///         test_int: 15
    ///         test_texture_name: (texture id)
    ///     }
    /// }
    /// ```
    /// For uniforms, the shader will read the types of uniforms in the shader, and
    /// use them.
    ///
    /// Matrices are not supported yet
    /// </summary>
    /// \see @ref cqsp::asset::ShaderDefinition for a most up to date version
    std::unique_ptr<ShaderDefinition> LoadShaderDefinition(
        cqsp::asset::VirtualMounter* mount, const std::string& path,
        const std::string& key, const Hjson::Value& hints);

    /// <summary>
    /// Loads a script directory.
    /// </summary>
    /// <param name="path"></param>
    /// <param name="hints"></param>
    /// <returns></returns>
    std::unique_ptr<TextDirectoryAsset> LoadScriptDirectory(VirtualMounter* mount,
                                                            const std::string& path,
                                                            const Hjson::Value& hints);

    /// <summary>
    /// Loads the asset specified in `path`
    /// </summary>
    /// <param name="type">Type of asset to load</param>
    /// <param name="path">Full virtual path of the asset</param>
    /// <param name="key">Key of the asset to be loaded</param>
    /// <param name="hints">Any hints to give to the loader</param>
    /// <returns></returns>
    std::unique_ptr<cqsp::asset::Asset> LoadAsset(const AssetType& type,
                   const std::string& path, const std::string& key,
                   const Hjson::Value& hints);

    ShaderProgram_t MakeShader(const std::string& key);

    /// <summary>
    /// Conducts checks to determine if the asset was loaded correctly. Wraps Load asset,
    /// and contains the same parameters
    /// </summary>
    /// <param name="package">Package to load into</param>
    void PlaceAsset(Package& package, const AssetType& type,
                    const std::string& path, const std::string& key,
                    const Hjson::Value& hints);

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
    /// It will look recursively and find all the resource.hjson files to load.
    /// <br>
    /// `resource.hjson` files consist of a hjson object.
    /// This is the syntax:
    /// ```
    /// {
    ///     test_asset: { # Asset key
    ///         path: # Path relative to the current resource.hjson file.
    ///         type: # Type of asset: &lt;none|texture|shader|hjson|text|model|font|cubemap|directory|audio&rt;
    ///         hints: {} # Object of whatever information you want when loading the asset.
    ///     }
    ///     // .. more assets can be specified, as long as they have a different key
    /// }
    /// ```
    /// <br>
    /// </summary>
    void LoadResources(Package& package, const std::string& path);

    /// <summary>
    /// Loads all the resources defined in the hjson `asset_value` in the hjson resource
    /// loading format.
    /// </summary>
    /// <param name="package">Package to load into</param>
    /// <param name="resource_mount_path">root path of the package</param>
    /// <param name="resource_file_path">Resource file path</param>
    /// <param name="asset_value">Hjson value to read from</param>
    void LoadResourceHjsonFile(Package& package,
                               const std::string& resource_mount_path,
                               const std::string& resource_file_path,
                               const Hjson::Value& asset_value);
    /// <summary>
    /// Defines a directory that contains hjson asset data.
    /// </summary>
    /// <param name="package"></param>
    /// <param name="path"></param>
    /// <param name="name"></param>
    /// <param name="mounter"></param>
    bool HjsonPrototypeDirectory(Package& package, const std::string& path, const std::string& name);

    /// <summary>
    /// Creates a virtual file system starting in path
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    IVirtualFileSystem* GetVfs(const std::string& path);

    /// <summary>
    /// List of all assets that are missing
    /// </summary>
    std::vector<std::string> missing_assets;
    /// <summary>
    /// Queue for @ref AssetPrototype when loading in frame.
    /// </summary>
    ThreadsafeQueue<QueueHolder> m_asset_queue;

    // All the assets that are to be loaded
    std::atomic_int max_loading;

    /// <summary>
    /// All the assets that have already been loaded
    /// </summary>
    std::atomic_int currentloading;
    /// <summary>
    /// The list of functions that are loading.
    /// </summary>
    /// \see @ref LoadScriptDirectory LoadCubemap LoadAudio LoadText LoadTexture LoadHjson LoadShader LoadFont
    std::map<AssetType, LoaderFunction> loading_functions;
    VirtualMounter mounter;
};
}  // namespace asset
}  // namespace cqsp
