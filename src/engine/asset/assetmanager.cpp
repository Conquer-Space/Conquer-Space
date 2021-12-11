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
#include "engine/asset/assetmanager.h"

#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <glad/glad.h>

#include <fstream>
#include <utility>
#include <algorithm>
#include <regex>
#include <iostream>
#include <filesystem>

#include "engine/audio/alaudioasset.h"
#include "engine/asset/vfs/nativevfs.h"
#include "common/util/paths.h"

#define CREATE_ASSET_LAMBDA(FuncName) [this] (cqsp::asset::VirtualMounter* mount,                   \
                                              const std::string& path, const std::string& key,      \
                                              const Hjson::Value& hints) {                          \
                                        return this->FuncName(mount, path, key, hints);                 \
                                      };

// Definition for prototypes
namespace cqsp::asset {
class ImagePrototype : public AssetPrototype {
 public:
    unsigned char* data;
    int width;
    int height;
    int components;

    asset::TextureLoadingOptions options;

    int GetPrototypeType() { return PrototypeType::TEXTURE; }
};

class CubemapPrototype : public AssetPrototype {
 public:
    std::vector<unsigned char*> data;
    int width;
    int height;
    int components;

    asset::TextureLoadingOptions options;

    int GetPrototypeType() { return PrototypeType::CUBEMAP; }
};

class ShaderPrototype : public AssetPrototype {
 public:
    std::string data;
    int type;
    Hjson::Value hints;

    int GetPrototypeType() { return PrototypeType::SHADER; }
};

class FontPrototype : public AssetPrototype {
 public:
    unsigned char* fontBuffer;
    int size;

    int GetPrototypeType() { return PrototypeType::FONT; }
};
}  // namespace cqsp::asset

bool cqsp::asset::Package::HasAsset(const char* asset) {
    return assets.count(asset) != 0;
}
bool cqsp::asset::Package::HasAsset(const std::string& asset) {
    return assets.count(asset) != 0;
}

void cqsp::asset::Package::ClearAssets() {
    for (auto a = assets.begin(); a != assets.end(); a++) {
        a->second.reset();
    }
    assets.clear();
}

using cqsp::asset::AssetLoader;

cqsp::asset::AssetManager::AssetManager() {}

cqsp::asset::ShaderProgram_t
cqsp::asset::AssetManager::MakeShader(const std::string& vert, const std::string& frag) {
    return std::make_shared<ShaderProgram>(*GetAsset<cqsp::asset::Shader>(vert.c_str()),
                                    *GetAsset<cqsp::asset::Shader>(frag.c_str()));
}

void cqsp::asset::AssetManager::LoadDefaultTexture() {
    unsigned char texture_bytes[] = {
        0, 0, 0, 255, 0, 255,
        0, 0, // These two padding bytes are needed for some reason. Opengl doesn't like 2x2 images
        255, 0, 255, 0, 0, 0
    };

    unsigned char* buffer = &texture_bytes[0];
    asset::TextureLoadingOptions f;
    f.mag_filter = true;
    asset::CreateTexture(empty_texture, buffer, 2, 2, 3, f);
}

void cqsp::asset::AssetManager::ClearAssets() {
    for (auto a = packages.begin(); a != packages.end(); a++) {
        a->second->ClearAssets();
    }
    packages.clear();
}

void cqsp::asset::AssetManager::SaveModList() {
    Hjson::Value enabled_mods;
    // Load the enabled mods, and write to the file. then exit game.
    for (auto it = m_package_prototype_list.begin(); it != m_package_prototype_list.end(); it++) {
        enabled_mods[it->second.name] = it->second.enabled;
    }
    // Write to file
    std::string mods_path = (std::filesystem::path(common::util::GetCqspSavePath())/"mod.hjson").string();
    Hjson::MarshalToFile(enabled_mods, mods_path);
    SPDLOG_INFO("Writing mods");
}

cqsp::asset::AssetLoader::AssetLoader() {
    loading_functions[AssetType::TEXT] = CREATE_ASSET_LAMBDA(LoadText);
    loading_functions[AssetType::TEXTURE] = CREATE_ASSET_LAMBDA(LoadTexture);
    loading_functions[AssetType::TEXT_ARRAY] = CREATE_ASSET_LAMBDA(LoadTextDirectory);
    loading_functions[AssetType::HJSON] = CREATE_ASSET_LAMBDA(LoadHjson);
    loading_functions[AssetType::SHADER] = CREATE_ASSET_LAMBDA(LoadShader);
    loading_functions[AssetType::AUDIO] = CREATE_ASSET_LAMBDA(LoadAudio);
    loading_functions[AssetType::CUBEMAP] = CREATE_ASSET_LAMBDA(LoadCubemap);
    loading_functions[AssetType::FONT] = CREATE_ASSET_LAMBDA(LoadFont);
}

namespace cqspa = cqsp::asset;

void cqsp::asset::AssetLoader::LoadMods() {
    // Load enabled mods
    // Load core
    std::filesystem::path data_path(cqsp::common::util::GetCqspDataPath());
    std::filesystem::recursive_directory_iterator it(data_path);

    // Load mods from the document folders
    // Find documents folder and load the information about the files

    // Keep track of all mods so that we can ensure that all are loaded
    Hjson::Value all_mods;

    // Some lambda things to keep things less cluttered and simpler
    auto mod_load = [&](const PackagePrototype &package) {
        // Add to package prototypes
        if (!package.name.empty()) {
            SPDLOG_INFO("Added mod prototype {}", package.name);
            manager->m_package_prototype_list[package.name] = package;
            all_mods[package.name] = false;
        }
    };

    SPDLOG_INFO("Loading potential mods");

    // Load core
    mod_load(LoadModPrototype((data_path/"core").string()));

    // Enable core by default
    all_mods["core"] = true;

    // Load other packages
    std::filesystem::path save_path(cqsp::common::util::GetCqspSavePath());
    std::filesystem::path mods_folder = save_path / "mods";
    if (!std::filesystem::exists(mods_folder)) {
        std::filesystem::create_directories(mods_folder);
    }

    // Load all the mods
    std::filesystem::directory_iterator mods_folder_iterator(mods_folder);
    for (auto mod_element : mods_folder_iterator) {
        mod_load(LoadModPrototype(mod_element.path().string()));
    }
    SPDLOG_INFO("Loaded mods folder");

    // Get loaded mods
    // If it doesn't exist, then create it
    std::filesystem::path mods_path(GetModFilePath());
    if (!std::filesystem::exists(mods_path)) {
        Hjson::Value mods;
        // Nothing in the mods
        Hjson::MarshalToFile(mods, mods_path.string());
    }

    Hjson::Value mods = Hjson::UnmarshalFromFile(mods_path.string());
    // Apply the current mods to the hjson so that we can see what mods are to be loaded
    mods = Hjson::Merge(all_mods, mods);

    // Load the mods that are to be loaded
    for (auto it : mods) {
        manager->m_package_prototype_list[it.first].enabled = static_cast<bool>(it.second);
    }

    // Load all the packages
    for (auto it : manager->m_package_prototype_list) {
        // Get the thing
        auto package = LoadPackage(it.second.path);
        if (package == nullptr) {
            continue;
        }
        manager->packages[it.first] = std::move(package);
    }
}

std::string cqsp::asset::AssetLoader::GetModFilePath() {
    return (std::filesystem::path(cqsp::common::util::GetCqspSavePath())/"mod.hjson").string();
}

cqsp::asset::PackagePrototype cqsp::asset::AssetLoader::LoadModPrototype(const std::string& path_string) {
    // Load the info.hjson
    std::filesystem::path package_path(path_string);
    IVirtualFileSystem* vfs = GetVfs(path_string);

    if (!vfs->IsFile("info.hjson")) {
        SPDLOG_INFO("Mod prototype unable to be loaded from {}", path_string);
        return PackagePrototype();
    }
    // Load the file
    
    Hjson::Value mod_info = Hjson::Unmarshal(
        cqsp::asset::ReadAllFromVFileToString(vfs->Open("info.hjson").get()));
    // Get the info of the file
    // Add core
    PackagePrototype prototype;
    try {
        prototype.name = mod_info["name"].to_string();
        prototype.version = mod_info["version"].to_string();
        prototype.title = mod_info["title"].to_string();
        prototype.author = mod_info["author"].to_string();
        prototype.path = package_path.string();
    } catch (Hjson::index_out_of_bounds &ex) {
        // Don't load the mod
        SPDLOG_INFO("Hjson::index_out_of_bounds: {}", ex.what());
    }

    // Free memory
    delete vfs;
    return prototype;
}

void cqsp::asset::AssetLoader::LoadHjsonDirectory(Package& package, std::string path, std::string key) {
    Hjson::Value hints;
    std::unique_ptr<HjsonAsset> asset = std::make_unique<HjsonAsset>();
    // Verify that the directory exists
    if (std::filesystem::exists(std::filesystem::path(path)) &&
                std::filesystem::is_directory(std::filesystem::path(path))) {
        LoadHjsonDir(path, asset->data, hints);
        package.assets[key] = std::move(asset);
    }
}

std::unique_ptr<cqsp::asset::Package> cqsp::asset::AssetLoader::LoadPackage(std::string path) {
    // Load into filesystem
    // Load the assets of a package specified by a path
    // First load info.hjson, the info path of the file.
    std::filesystem::path package_path(path);

    // Mount the path
    IVirtualFileSystem* vfs = GetVfs(package_path.string());

    // Mount package path
    SPDLOG_INFO("Loading package {}", package_path.string());
    // Read info.hjson from package
    auto info_file = vfs->Open("info.hjson");
    if (info_file == nullptr) {
        SPDLOG_ERROR("Failed to load package {}", package_path.string());
        return nullptr;
    }

    // Read the data
    Hjson::Value info = Hjson::Unmarshal(ReadAllFromVFileToString(info_file.get()));

    // Place into string
    // This will also serve as the namespace name, so no spaces, periods, semicolons please
    std::unique_ptr<Package> package = std::make_unique<Package>();
    package->name = info["name"].to_string();
    package->version = info["version"].to_string();
    package->title = info["title"].to_string();
    package->author = info["author"].to_string();

    // Mount to name
    std::string mount_point = package->name;
    mounter.AddMountPoint(mount_point.c_str(), vfs);

    SPDLOG_INFO("Mounted package {}", package->name);

    // Load dependencies
    // Now load the 'important' folders
    std::filesystem::path script_path(package_path / "scripts");
    // Check if files exist
    // Load scripts
    if (mounter.IsDirectory(mount_point, "scripts") && mounter.IsFile(mount_point, "scripts/base.lua")) {
        // Load base.lua for the base folder
        package->assets["base"] = LoadText(&mounter, mount_point + "/scripts/base.lua", "base", Hjson::Value());
        package->assets["scripts"] = LoadScriptDirectory((package_path / "scripts").string(), Hjson::Value());
        SPDLOG_INFO("Loaded scripts");
    } else {
        SPDLOG_INFO("No script file for package {}", package->name);
    }

    // Load a few other hjson folders.
    // So the folders we have to keep track off are the goods and recipes
    HjsonPrototypeDirectory(*package, fmt::format("{}/data/goods", mount_point), "goods");
    HjsonPrototypeDirectory(*package, fmt::format("{}/data/recipes", mount_point), "recipes");

    SPDLOG_INFO("Loaded prototype directories");

    // Then load all the other assets
    // Load resource.hjsons
    LoadResources(*package, package->name);
    return package;
}

void cqsp::asset::AssetLoader::LoadAsset(Package& package, const AssetType& type, const std::string& path,
                                        const std::string& key, const Hjson::Value& hints) {
    switch (type) {
        case AssetType::TEXTURE:
        {
        package.assets[key] = LoadTexture(key, path, hints);
        break;
        }
        case AssetType::SHADER:
        {
        std::ifstream asset_stream(path, std::ios::binary);
        package.assets[key] = LoadShader(key, asset_stream, hints);
        break;
        }
        case AssetType::HJSON:
        {
        // Check for errors
        auto p = LoadHjson(path, hints);
        if (p != nullptr) {
            package.assets[key] = std::move(p);
        }
        break;
        }
        case AssetType::TEXT:
        {
        std::ifstream asset_stream(path);
        package.assets[key] = LoadText(asset_stream, hints);
        break;
        }
        case AssetType::TEXT_ARRAY:
        {
        break;
        // This is not really needed.
        package.assets[key] = LoadTextDirectory(path, hints);
        }
        case AssetType::MODEL:
        break;
        case AssetType::CUBEMAP:
        {
        std::ifstream asset_stream(path);
        package.assets[key] = LoadCubemap(key, path, asset_stream, hints);
        break;
        }
        case AssetType::FONT:
        {
        std::ifstream asset_stream(path, std::ios::binary);
        package.assets[key] = LoadFont(key, asset_stream, hints);
        break;
        }
        case AssetType::AUDIO:
        {
        std::ifstream asset_stream(path, std::ios::binary);
        package.assets[key] = cqsp::asset::LoadOgg(asset_stream);
        break;
        }
        break;
        case AssetType::NONE:
        default:
        break;
    }
}


std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadAsset2(
                                          const AssetType& type,
                                          const std::string& path,
                                          const std::string& key,
                                          const Hjson::Value& hints) {
    // Load asset
    if (loading_functions.find(type) == loading_functions.end()) {
        SPDLOG_WARN("{} asset loading not support yet", ToString(type));
        return nullptr;
    }
    return std::move(loading_functions[type](&mounter, path, key, hints));
}

void cqsp::asset::AssetLoader::PlaceAsset(Package& package,
                                          const AssetType& type,
                                          const std::string& path,
                                          const std::string& key,
                                          const Hjson::Value& hints) {
    auto asset = LoadAsset2(type, path, key, hints);
    if (asset == nullptr) {
        SPDLOG_WARN("Asset {} was not loaded properly", key);
        return;
    }
    package.assets[key] = std::move(asset);
}

void cqsp::asset::AssetLoader::LoadAsset(Package& package,
                                         const std::string& type,
                                         const std::string& path,
                                         const std::string& key,
                                         const Hjson::Value& hints) {
    LoadAsset(package, FromString(type), path, key, hints);
}

std::unique_ptr<cqsp::asset::Texture> cqsp::asset::AssetLoader::LoadTexture(const std::string& key,
    const std::string & filePath, const Hjson::Value & hints) {
    // Get the things
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();
    //return asset::Texture();
    ImagePrototype* prototype = new ImagePrototype();
    prototype->asset = texture.get();
    prototype->key = key;
    // Load image
    Hjson::Value pixellated = hints["magfilter"];

    if (pixellated.defined() && pixellated.type() == Hjson::Type::Bool) {
        // Then it's pixellated, or closest magfilter
        prototype->options.mag_filter = static_cast<bool>(pixellated);
    } else {
        // it's linear mag filter by default
        prototype->options.mag_filter = false;
    }

    // Read entire file
    std::ifstream input(filePath, std::ios::binary);
    input.seekg(0, std::ios::end);
    // Get size
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    char* data = new char[size];
    input.read(data, size);
    unsigned char* d = (unsigned char *) data;

    // Load from file
    prototype->data = stbi_load_from_memory(d, size, &prototype->width, &prototype->height,
                           &prototype->components, 0);

    if (prototype->data) {
        QueueHolder holder(prototype);

        m_asset_queue.push(holder);
    } else {
        SPDLOG_INFO("Failed to load image {}", key);
        delete prototype;
    }
    return texture;
}

void cqsp::asset::AssetLoader::BuildNextAsset() {
    if (m_asset_queue.size() == 0) {
        return;
    }

    auto value = m_asset_queue.pop();
    if (!value.has_value()) {
        return;
    }
    QueueHolder temp = *value;
    std::string key = temp.prototype->key;
    switch (temp.prototype->GetPrototypeType()) {
        case PrototypeType::TEXTURE:
        {
        ImagePrototype* texture_prototype = dynamic_cast<ImagePrototype*>(temp.prototype);
        Texture* asset = dynamic_cast<Texture*>(texture_prototype->asset);
        asset::CreateTexture(*asset,
                            texture_prototype->data,
                            texture_prototype->width,
                            texture_prototype->height,
                            texture_prototype->components,
                            texture_prototype->options);

        stbi_image_free(texture_prototype->data);
        break;
        }
        case PrototypeType::SHADER:
        {
        ShaderPrototype* shader = dynamic_cast<ShaderPrototype*>(temp.prototype);
        Shader* asset = dynamic_cast<Shader*>(shader->asset);
        try  {
            int shaderId = asset::LoadShader(shader->data, shader->type);
            asset->id = shaderId;
        } catch (std::runtime_error &error) {
            SPDLOG_WARN("Exception in loading shader {}: {}", shader->key, error.what());
        }
        break;
        }
        case PrototypeType::FONT:
        {
        FontPrototype* prototype = dynamic_cast<FontPrototype*>(temp.prototype);
        Font* asset = dynamic_cast<Font*>(prototype->asset);

        asset::LoadFont(*asset, prototype->fontBuffer, prototype->size);
        }
        break;
        case PrototypeType::CUBEMAP:
        {
        CubemapPrototype* prototype = dynamic_cast<CubemapPrototype*>(temp.prototype);
        Texture* asset = dynamic_cast<Texture*>(prototype->asset);
        asset::LoadCubemap(*asset,
                            prototype->data,
                            prototype->width,
                            prototype->height,
                            prototype->components,
                            prototype->options);
        }
        break;
    }

    // Free memory
    delete temp.prototype;
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadText(cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    if (!mount->IsFile(path)) {
        return nullptr;
    }
    std::unique_ptr<cqspa::TextAsset> asset = std::make_unique<cqspa::TextAsset>();
    auto file = mount->Open(path.c_str());
    int size = file->Size();
    asset->data = ReadAllFromVFileToString(file.get());
    return std::move(asset);
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadTextDirectory(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    if (!mount->IsDirectory(path)) {
        return nullptr;
    }
    std::unique_ptr<cqspa::TextDirectoryAsset> asset = std::make_unique<cqspa::TextDirectoryAsset>();
    auto dir = mount->OpenDirectory(path.c_str());
    int size = dir->GetSize();
    for (int i = 0; i < size; i++) {
        auto file = dir->GetFile(i, FileModes::Binary);
        uint64_t file_size = file->Size();
        uint8_t* buffer = ReadAllFromVFile(file.get());
        cqsp::asset::PathedTextAsset asset_data(reinterpret_cast<char*>(buffer), file_size);
        asset_data.path = path;
    }
    return std::move(asset);
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadTexture(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();

    // Create prototype
    ImagePrototype* prototype = new ImagePrototype();
    prototype->asset = texture.get();
    prototype->key = key;

    Hjson::Value pixellated = hints["magfilter"];
    // Check type of magfilter, sometimes you want a different type
    if (pixellated.defined() && pixellated.type() == Hjson::Type::Bool) {
        prototype->options.mag_filter = static_cast<bool>(pixellated);
    } else {
        // It's linear mag filter by default
        prototype->options.mag_filter = false;
    }

    auto file = mount->Open(path.c_str(), FileModes::Binary);
    uint64_t file_size = file->Size();
    uint8_t* buffer = ReadAllFromVFile(file.get());
    prototype->data = stbi_load_from_memory(buffer, file_size, &prototype->width, &prototype->height,
                           &prototype->components, 0);

    if (prototype->data) {
        QueueHolder holder(prototype);

        m_asset_queue.push(holder);
    } else {
        SPDLOG_ERROR("Failed to load image {}", key);
        delete prototype;
        return nullptr;
    }
    return std::move(texture);
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadHjson(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    std::unique_ptr<cqspa::HjsonAsset> asset = std::make_unique<cqspa::HjsonAsset>();

    Hjson::DecoderOptions dec_opt;
    dec_opt.comments = false;

    // Load a directory if it's a directory
    if (mount->IsDirectory(path)) {
        // Load and append to assets.
        auto dir = mount->OpenDirectory(path);
        for (int i = 0; i < dir->GetSize(); i++) {
            auto file = dir->GetFile(i);
            Hjson::Value result;
            // Since it's a directory, we will assume it's an array, and push back the values.
            try {
                result = Hjson::Unmarshal(ReadAllFromVFileToString(file.get()), dec_opt);
                if (result.type() == Hjson::Type::Vector) {
                    // Append everything
                    for(int k = 0; k < result.size(); k++) {
                        asset->data.push_back(result[k]);
                    }
                } else {
                    SPDLOG_ERROR("Failed to load hjson {}: it needs to be a array", file->Path());
                }
            } catch (Hjson::syntax_error& ex) {
                SPDLOG_ERROR("Failed to load hjson {}: {}", file->Path(), ex.what());
            }
        }
    } else {
        auto file = mount->Open(path.c_str());
        // Read the file
        try {
            asset->data = Hjson::Unmarshal(ReadAllFromVFileToString(file.get()), dec_opt);
        } catch (Hjson::syntax_error& ex) {
            SPDLOG_ERROR("Failed to load hjson {}: {}", path, ex.what());
        }
    }
    return asset;
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadShader(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    if (!mount->IsFile(path)) {
        return nullptr;
    }
    std::unique_ptr<Shader> shader = std::make_unique<Shader>();
    // Get shader type
    std::string type = hints["type"];
    int shader_type = -1;
    if (type == "frag") {
        shader_type = GL_FRAGMENT_SHADER;
    } else if (type == "vert") {
        shader_type = GL_VERTEX_SHADER;
    } else {
        // Abort, because this is a dud.
        SPDLOG_WARN("Unsupport shader type: {}", key);
        return nullptr;
    }

    ShaderPrototype* prototype = new ShaderPrototype();

    prototype->key = key;
    prototype->asset = shader.get();
    prototype->type = shader_type;
    prototype->hints = hints;

    prototype->data = ReadAllFromVFileToString(mount->Open(path).get());

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);

    return std::move(shader);
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadFont(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    if (!mount->IsFile(path)) {
        return nullptr;
    }

    std::unique_ptr<Font> asset = std::make_unique<Font>();
    auto file = mount->Open(path);
    uint8_t* bytes = ReadAllFromVFile(file.get());

    FontPrototype* prototype = new FontPrototype();
    prototype->fontBuffer = bytes;
    prototype->size = file->Size();
    prototype->key = key;
    prototype->asset = asset.get();

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);

    return std::move(asset);
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadAudio(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    // Load audio asset
    if (!mount->IsFile(path)) {
        return nullptr;
    }
    auto file = mount->Open(path);
    uint8_t* data = ReadAllFromVFile(file.get());
    auto asset = LoadOgg(data, file->Size());

    return std::move(asset);
}

std::unique_ptr<cqsp::asset::Asset> cqsp::asset::AssetLoader::LoadCubemap(
    cqsp::asset::VirtualMounter* mount, const std::string& path,
    const std::string& key, const Hjson::Value& hints) {
    // Load cubemap data
    std::unique_ptr<Texture> asset = std::make_unique<Texture>();

    // Read file, which will be hjson, and load those files too
    Hjson::Value images_hjson;
    auto hjson_file = mount->Open(path);
    images_hjson = Hjson::Unmarshal(ReadAllFromVFileToString(hjson_file.get()));

    CubemapPrototype* prototype = new CubemapPrototype();

    prototype->key = key;

    std::string parent = GetParentPath(path);

    if (images_hjson.size() != 6) {
        SPDLOG_WARN("Cubemap {} does not have enough faces defined", key);
        return nullptr;
    }
    for (int i = 0; i < images_hjson.size(); i++) {
        std::string image_path = parent + "/" + images_hjson[i];
        if (!mount->IsFile(image_path)) {
            SPDLOG_WARN("Cubemap {} has missing faces!", key);
            // Free memory
            for (auto texture : prototype->data) {
                // free the things
                stbi_image_free(texture);
            }
            return nullptr;
        }
        auto file = mount->Open(image_path);
        auto file_data = ReadAllFromVFile(file.get());
        unsigned char* image_data = stbi_load_from_memory(file_data, file->Size(), &prototype->width, &prototype->height,
                           &prototype->components, 0);

        prototype->data.push_back(image_data);
    }
    prototype->asset = asset.get();

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);
    return asset;
}

std::unique_ptr<cqspa::TextAsset> cqsp::asset::AssetLoader::LoadText(std::istream& asset_stream,
                                                                    const Hjson::Value& hints) {
    std::unique_ptr<cqspa::TextAsset> asset = std::make_unique<cqspa::TextAsset>();

    // Load from string
    std::string asset_data{std::istreambuf_iterator<char>{asset_stream},
                            std::istreambuf_iterator<char>()};

    asset->data = asset_data;
    return std::move(asset);
}

std::unique_ptr<cqspa::HjsonAsset> cqspa::AssetLoader::LoadHjson(const std::string &path,
                                    const Hjson::Value& hints) {
    std::unique_ptr<cqspa::HjsonAsset> asset =
        std::make_unique<cqspa::HjsonAsset>();
        // Load a directory if it's a directory
    if (std::filesystem::is_directory(path)) {
        // Load and append to assets.
        Hjson::Value data;
        LoadHjsonDir(path, data, hints);
        asset->data = data;
        return asset;
    } else {
        std::ifstream asset_stream(path);
        Hjson::DecoderOptions decOpt;
        try {
            decOpt.comments = false;
            asset_stream >> Hjson::StreamDecoder(asset->data, decOpt);
        } catch (Hjson::syntax_error& ex) {
            SPDLOG_ERROR("Failed to load hjson {}: ", path, ex.what());
            return nullptr;
        }
    }
    return asset;
}

std::unique_ptr<cqsp::asset::TextDirectoryAsset>
AssetLoader::LoadScriptDirectory(const std::string& path, const Hjson::Value& hints) {
    std::filesystem::path root(path);
    auto asset = std::make_unique<asset::TextDirectoryAsset>();
    LoadDirectory(path, [&](std::string path) {
        std::filesystem::directory_entry sub_path(path);
        // Ensure it's a lua file
        if (!sub_path.is_regular_file() && sub_path.path().extension() != "lua" ||
            std::filesystem::relative(sub_path.path(), root).filename() == "base.lua") {  // Ignore base.lua
            return;
        }

        std::ifstream asset_stream(sub_path.path().string());
        cqsp::asset::PathedTextAsset asset_data{std::istreambuf_iterator<char>{asset_stream},
                        std::istreambuf_iterator<char>()};

        // Get the path, and also remove the .lua extension so that it's easier to access it
        asset_data.path = std::filesystem::relative(sub_path.path(),
                                    root).replace_extension("").string();
        // Replace the data path so that the name will be using dots, and we don't need to care
        // about file separators
        // So requiring a lua file (that is the purpose for this), will look like
        // require("test.abc")
        // to require a file called test/abc.lua
        std::replace(asset_data.path.begin(), asset_data.path.end(),
                            static_cast<char>(std::filesystem::path::preferred_separator), '.');
        asset->paths[asset_data.path] = asset_data;
    });

    return asset;
}

// This is essentially all for lua
std::unique_ptr<cqsp::asset::TextDirectoryAsset>
cqsp::asset::AssetLoader::LoadTextDirectory(const std::string& path, const Hjson::Value& hints) {
    auto asset = std::make_unique<asset::TextDirectoryAsset>();
    LoadDirectory(path, [&](std::string path) {
        std::filesystem::directory_entry sub_path(path);
        if (!sub_path.is_regular_file()) {
            return;
        }

        std::ifstream asset_stream(sub_path.path().string());
        cqsp::asset::PathedTextAsset asset_data{std::istreambuf_iterator<char>{asset_stream},
                        std::istreambuf_iterator<char>()};

        asset->paths[asset_data.path] = asset_data;
    });
    return asset;
}

void cqspa::AssetLoader::LoadHjsonFromArray(std::istream &asset_stream, Hjson::Value& value,
                                    const Hjson::Value& hints) {
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    Hjson::Value data;
    asset_stream >> Hjson::StreamDecoder(data, decOpt);
    // Append the values, because it's in a vector
    for (int i = 0; i < data.size(); i++) {
        value.push_back(data[i]);
    }
}

void AssetLoader::LoadHjsonDir(const std::string& path, Hjson::Value& value, const Hjson::Value& hints) {
    LoadDirectory(path, [&](std::string entry) {
        std::ifstream asset_stream(entry);
        if (!asset_stream.good()) {
            return;
        }
        LoadHjsonFromArray(asset_stream, value, hints);
    });
}

std::unique_ptr<cqsp::asset::Shader> cqspa::AssetLoader::LoadShader(const std::string& key, std::istream& asset_stream,
                                                const Hjson::Value& hints) {
    std::unique_ptr<Shader> shader = std::make_unique<Shader>();
    // Get shader type
    std::string type = hints["type"];
    int shader_type = -1;
    if (type == "frag") {
        shader_type = GL_FRAGMENT_SHADER;
    } else if (type == "vert") {
        shader_type = GL_VERTEX_SHADER;
    } else {
        // Abort, because this is a dud.
        return shader;
    }

    ShaderPrototype* prototype = new ShaderPrototype();

    prototype->key = key;
    prototype->asset = shader.get();
    prototype->type = shader_type;
    prototype->hints = hints;
    std::istreambuf_iterator<char> eos;
    std::string s(std::istreambuf_iterator<char>(asset_stream), eos);
    prototype->data = s;

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);

    return shader;
}

std::unique_ptr<cqsp::asset::Font> cqsp::asset::AssetLoader::LoadFont(const std::string& key,
    std::istream& asset_stream, const Hjson::Value& hints) {
    std::unique_ptr<Font> asset = std::make_unique<Font>();
    asset_stream.seekg(0, std::ios::end);
    std::fstream::pos_type fontFileSize = asset_stream.tellg();
    asset_stream.seekg(0);
    unsigned char *fontBuffer = new unsigned char[fontFileSize];
    asset_stream.read(reinterpret_cast<char*>(fontBuffer), fontFileSize);

    FontPrototype* prototype = new FontPrototype();
    prototype->fontBuffer = fontBuffer;
    prototype->size = fontFileSize;
    prototype->key = key;
    prototype->asset = asset.get();

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);

    return asset;
}

std::unique_ptr<cqsp::asset::Texture> cqsp::asset::AssetLoader::LoadCubemap(const std::string& key,
                                                    const std::string &path,
                                                    std::istream &asset_stream,
                                                    const Hjson::Value& hints) {
    std::unique_ptr<Texture> asset = std::make_unique<Texture>();

    // Read file, which will be hjson, and load those files too
    Hjson::Value images;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    asset_stream >> Hjson::StreamDecoder(images, decOpt);

    CubemapPrototype* prototype = new CubemapPrototype();

    prototype->key = key;

    std::filesystem::path p(path);
    std::filesystem::path dir = p.parent_path();

    for (int i = 0; i < images.size(); i++) {
        std::string filePath = dir.string() + "/" + images[i];

        unsigned char* data = stbi_load(filePath.c_str(), &prototype->width, &prototype->height,
                &prototype->components, 0);

        prototype->data.push_back(data);
    }
    prototype->asset = asset.get();

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);
    return asset;
}

void AssetLoader::LoadDirectory(std::string path, std::function<void(std::string)> file) {
    auto iterator = std::filesystem::recursive_directory_iterator(path);
    int count = std::distance(iterator, std::filesystem::recursive_directory_iterator());
    max_loading += count;
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path)) {
        file(dirEntry.path().string());
        currentloading++;
    }
}

void cqsp::asset::AssetLoader::LoadResources(Package& package, const std::string& package_mount_path) {
    // Load the package
    // Open the root directory
    auto directory = mounter.OpenDirectory(package_mount_path + "/");
    SPDLOG_INFO("Loading {}", package_mount_path);
    for (int i = 0; i < directory->GetSize(); i++) {
        auto resource_file = directory->GetFile(i);
        // Get the path
        if (GetFilename(resource_file->Path()) != "resource.hjson") {
            continue;
        }

        // Load the particular asset folder
        // Open the file
        Hjson::DecoderOptions dec_opt;
        dec_opt.comments = false;
        dec_opt.duplicateKeyException = true;
        std::string asset_data = ReadAllFromVFileToString(resource_file.get());
        Hjson::Value asset_value;

        // Try to load and check for duplicate options, sadly hjson doesn't provide good
        // ways to see which keys are duplicated, except by exception, so we'll have
        // to do this as a hack for now
        try {
            asset_value = Hjson::Unmarshal(asset_data, dec_opt);
        } catch (Hjson::syntax_error &se) {
            SPDLOG_WARN(se.what());
            // Then try again without the options
            dec_opt.duplicateKeyException = false;
            asset_value = Hjson::Unmarshal(asset_data, dec_opt);
        }

        max_loading += asset_value.size();
        for (const auto [key, val] : asset_value) {
            SPDLOG_TRACE("Loading asset {}", key);

            std::string type = val["type"];

            // Get the file
            std::string path;
            if (GetParentPath(resource_file->Path()).empty()) {
                path = package_mount_path + "/" + val["path"].to_string();
            } else {
                path = package_mount_path + "/" + GetParentPath(resource_file->Path()) + "/" + val["path"].to_string();
            }
            SPDLOG_TRACE("Loading path {}", path);

            if (!mounter.Exists(path)) {
                SPDLOG_WARN("Cannot find asset {} at {}", key, path);
                // Check if it's required
                if (!val["required"].empty() && val["required"]) {
                    // Then required
                    SPDLOG_CRITICAL("Cannot find critical resource {}, exiting", key);
                    missing_assets.push_back(key);
                }
                continue;
            }
            // Put in core namespace, I guess
            // Load package
            PlaceAsset(package, FromString(type), path, std::string(key), val["hints"]);
            currentloading++;
        }
    }
}

bool cqsp::asset::AssetLoader::HjsonPrototypeDirectory(
    Package& package, const std::string& path, const std::string& name) {
    if (!mounter.IsDirectory(path)) {
        return false;
    }
    package.assets[name] = LoadHjson(&mounter, path, name, Hjson::Value());
    return true;
}

cqsp::asset::IVirtualFileSystem* cqsp::asset::AssetLoader::GetVfs(const std::string& path) {
    // Return native filesystem for now.
    return new NativeFileSystem(path.c_str());
}
