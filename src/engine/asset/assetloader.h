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

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "engine/asset/assetprototype.h"
#include "engine/asset/package.h"
#include "engine/asset/packageprototype.h"
#include "engine/asset/textasset.h"
#include "engine/asset/vfs/vfs.h"
#include "engine/graphics/shader.h"
#include "engine/util/threadsafequeue.h"

namespace cqsp::asset {
class QueueHolder {
 public:
    QueueHolder() { prototype = nullptr; }
    explicit QueueHolder(AssetPrototype* type) : prototype(type) {}

    AssetPrototype* prototype;
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
    std::unique_ptr<Package> LoadPackage(const std::string& path);

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

    typedef std::function<std::unique_ptr<Asset>(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                 const std::string& key, const Hjson::Value& hints)>
        LoaderFunction;

 private:
    std::optional<PackagePrototype> LoadModPrototype(const std::string&);

    /// <summary>
    /// Pretty straightforward, loads a text file into a string.
    ///
    /// This has no hints
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadText(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                 const std::string& key, const Hjson::Value& hints);
    /// <summary>
    /// Loads a directory of text files into a map of strings keyed by their relative path to the
    /// resource.hjson file.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadTextDirectory(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                          const std::string& key, const Hjson::Value& hints);

    /// <summary>
    /// Textures have one hint, the `magfilter` hint. If it is present, and set to true, it will enable
    /// closest magfilter, which will make the texture look pixellated.
    /// If it is not present, then it will be linear mag.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadTexture(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                    const std::string& key, const Hjson::Value& hints);

    /// <summary>
    /// Loads binary data straight from the file.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadBinaryAsset(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                        const std::string& key, const Hjson::Value& hints);

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
    std::unique_ptr<cqsp::asset::Asset> LoadHjson(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                  const std::string& key, const Hjson::Value& hints);

    /// <summary>
    /// Shaders have one option, the `type` hint, to specify what type of shader it is.
    /// We have two so far, the `frag` option for a fragment shader, and `vert` for a vertex shader.
    /// We do not have support for compute and geometry shaders, but we may add support for that in the future.
    /// </summary>
    std::unique_ptr<cqsp::asset::Asset> LoadShader(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                   const std::string& key, const Hjson::Value& hints);
    /// <summary>
    /// Just specify a .ttf file, and it will load it into an opengl texture font object.
    /// </summary>
    /// This is for opengl rendering.
    ///
    /// Because each UI library has their differing implementation of fonts, to add
    /// fonts to the UI, do not use a `resource.hjson`. You need to go to `binaries/data/core/gfx/fonts.hjson`
    /// to alter and add fonts for the UI.
    std::unique_ptr<cqsp::asset::Asset> LoadFont(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                 const std::string& key, const Hjson::Value& hints);
    /// <summary>
    /// Only ogg files are supported for now.
    /// </summary>
    /// If you're looking to add a music file, do not add it by the 'normal' way. This is for performance
    /// reasons.
    /// Go to `binaries/data/core/music/readme.txt` for further reading.
    std::unique_ptr<cqsp::asset::Asset> LoadAudio(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                  const std::string& key, const Hjson::Value& hints);

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
    std::unique_ptr<cqsp::asset::Asset> LoadCubemap(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                    const std::string& key, const Hjson::Value& hints);

    std::unique_ptr<cqsp::asset::Asset> LoadModel(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                  const std::string& key, const Hjson::Value& hints);

    /// <summary>
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
    std::unique_ptr<ShaderDefinition> LoadShaderDefinition(cqsp::asset::VirtualMounter* mount, const std::string& path,
                                                           const std::string& key, const Hjson::Value& hints);

    /// <summary>
    /// Loads a script directory.
    /// </summary>
    /// <param name="path"></param>
    /// <param name="hints"></param>
    /// <returns></returns>
    std::unique_ptr<TextDirectoryAsset> LoadScriptDirectory(VirtualMounter* mount, const std::string& path,
                                                            const Hjson::Value& hints);

    /// <summary>
    /// Loads the asset specified in `path`
    /// </summary>
    /// <param name="type">Type of asset to load</param>
    /// <param name="path">Full virtual path of the asset</param>
    /// <param name="key">Key of the asset to be loaded</param>
    /// <param name="hints">Any hints to give to the loader</param>
    /// <returns></returns>
    std::unique_ptr<cqsp::asset::Asset> LoadAsset(const AssetType& type, const std::string& path,
                                                  const std::string& key, const Hjson::Value& hints);

    /// <summary>
    /// Conducts checks to determine if the asset was loaded correctly. Wraps Load asset,
    /// and contains the same parameters
    /// </summary>
    /// <param name="package">Package to load into</param>
    void PlaceAsset(Package& package, const AssetType& type, const std::string& path, const std::string& key,
                    const Hjson::Value& hints);

    /// <summary>
    /// Loads any type of directory, and executes the function for every single file.
    /// <br>
    /// This is a helper function to add to calculate the number of assets loaded so far.
    /// </summary>
    /// <param name="path">Path of directory to read</param>
    /// <param name="file">Function pointer to do something with the path, and it takes the path of the
    /// asset as the parameter</param>
    void LoadDirectory(const std::string& path, const std::function<void(std::string)>& file);

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
    void LoadResourceHjsonFile(Package& package, const std::string& package_mount_path,
                               const std::string& resource_file_path, const Hjson::Value& asset_value);
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
    engine::ThreadsafeQueue<QueueHolder> m_asset_queue;

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
    std::map<AssetType, uint64_t> loading_times;
    VirtualMounter mounter;
};
}  // namespace cqsp::asset
