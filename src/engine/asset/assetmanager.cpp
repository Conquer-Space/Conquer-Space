/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <regex>
#include <utility>
#include <vector>

#include <tracy/Tracy.hpp>

#include "common/util/paths.h"
#include "engine/asset/vfs/nativevfs.h"
#include "engine/audio/alaudioasset.h"
#include "engine/enginelogger.h"

#define CREATE_ASSET_LAMBDA(FuncName)                                                                           \
    [this](VirtualMounter* mount, const std::string& path, const std::string& key, const Hjson::Value& hints) { \
        return this->FuncName(mount, path, key, hints);                                                         \
    };

// Definition for prototypes
namespace cqsp::asset {
namespace {
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
    std::vector<uint8_t> fontBuffer;
    int size;

    int GetPrototypeType() { return PrototypeType::FONT; }
};
}  // namespace

bool Package::HasAsset(const char* asset) { return assets.count(asset) != 0; }
bool Package::HasAsset(const std::string& asset) { return assets.count(asset) != 0; }

void Package::ClearAssets() {
    for (auto a = assets.begin(); a != assets.end(); a++) {
        a->second.reset();
    }
    assets.clear();
}

AssetManager::AssetManager() {}

ShaderProgram_t AssetManager::MakeShader(const std::string& vert, const std::string& frag) {
    return std::make_shared<ShaderProgram>(*GetAsset<Shader>(vert.c_str()), *GetAsset<Shader>(frag.c_str()));
}

ShaderProgram_t AssetManager::MakeShader(const std::string& vert, const std::string& frag, const std::string& geom) {
    return std::make_shared<ShaderProgram>(*GetAsset<Shader>(vert.c_str()), *GetAsset<Shader>(frag.c_str()),
                                           *GetAsset<Shader>(geom.c_str()));
}

std::string GetShaderCode(const std::string& identifier) { return std::string(); }

void MakeShader(const Hjson::Value& hjson) {
    // Load the shader values
    std::string vert = hjson["vert"].to_string();
    std::string frag = hjson["frag"].to_string();
    // Get the shader code somehow, but idk how

    Shader vert_shader(GetShaderCode(vert), ShaderType::VERT);
    Shader frag_shader(GetShaderCode(frag), ShaderType::FRAG);
    // Create the shader
    ShaderProgram_t shader = MakeShaderProgram(vert_shader, frag_shader);
    // Initial values
    Hjson::Value initial = hjson["initial"];
    for (auto value : initial) {
        switch (value.second.type()) {
            case Hjson::Type::Int64:
            case Hjson::Type::Double: {
                shader->Set(value.first, (float)value.second.to_double());
                break;
            }
            case Hjson::Type::Vector: {
                // Check if matrix or vector
                switch (value.second.size()) {
                    case 2:
                        shader->Set(value.first, (float)value.second[0].to_double(),
                                    (float)value.second[1].to_double());
                        break;
                    case 3:
                        shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double(),
                                    (float)value.second[2].to_double());
                        break;
                    case 4:
                        shader->Set(value.first, (float)value.second[0].to_double(), (float)value.second[1].to_double(),
                                    (float)value.second[2].to_double(), (float)value.second[3].to_double());
                        break;
                }
                break;
            }

            case Hjson::Type::Undefined:
            case Hjson::Type::Bool:
            case Hjson::Type::Null:
            case Hjson::Type::Map:
            case Hjson::Type::String:
                break;
        }
    }
}

void AssetManager::LoadDefaultTexture() {
    unsigned char texture_bytes[] = {
        0,   0, 0,   255, 0, 255,
        0,   0,  // These two padding bytes are needed for some reason. Opengl doesn't like 2x2 images
        255, 0, 255, 0,   0, 0};

    asset::TextureLoadingOptions f;
    f.mag_filter = true;
    asset::CreateTexture(empty_texture, texture_bytes, 2, 2, 3, f);
}

void AssetManager::ClearAssets() { ZoneScoped packages.clear(); }

void AssetManager::SaveModList() {
    Hjson::Value enabled_mods;
    // Load the enabled mods, and write to the file. then exit game.
    for (auto it = m_package_prototype_list.begin(); it != m_package_prototype_list.end(); it++) {
        enabled_mods[it->second.name] = it->second.enabled;
    }
    // Write to file
    std::string mods_path = (std::filesystem::path(common::util::GetCqspAppDataPath()) / "mod.hjson").string();
    Hjson::MarshalToFile(enabled_mods, mods_path);
    ENGINE_LOG_INFO("Writing mods");
}

AssetLoader::AssetLoader() {
    loading_functions[AssetType::TEXT] = CREATE_ASSET_LAMBDA(LoadText);
    loading_functions[AssetType::TEXTURE] = CREATE_ASSET_LAMBDA(LoadTexture);
    loading_functions[AssetType::TEXT_ARRAY] = CREATE_ASSET_LAMBDA(LoadTextDirectory);
    loading_functions[AssetType::HJSON] = CREATE_ASSET_LAMBDA(LoadHjson);
    loading_functions[AssetType::SHADER] = CREATE_ASSET_LAMBDA(LoadShader);
    loading_functions[AssetType::AUDIO] = CREATE_ASSET_LAMBDA(LoadAudio);
    loading_functions[AssetType::CUBEMAP] = CREATE_ASSET_LAMBDA(LoadCubemap);
    loading_functions[AssetType::FONT] = CREATE_ASSET_LAMBDA(LoadFont);
    loading_functions[AssetType::SHADER_DEFINITION] = CREATE_ASSET_LAMBDA(LoadShaderDefinition);
    loading_functions[AssetType::BINARY] = CREATE_ASSET_LAMBDA(LoadBinaryAsset);
}

namespace cqspa = cqsp::asset;

void AssetLoader::LoadMods() {
    ZoneScoped;
    // Load enabled mods
    // Load core
    std::filesystem::path data_path(cqsp::common::util::GetCqspDataPath());
    std::filesystem::recursive_directory_iterator it(data_path);

    // Load mods from the document folders
    // Find documents folder and load the information about the files

    // Keep track of all mods so that we can ensure that all are loaded
    Hjson::Value all_mods;

    // Some lambda things to keep things less cluttered and simpler
    auto mod_load = [&](const std::optional<PackagePrototype>& package) {
        // Add to package prototypes
        if (!package) {
            ENGINE_LOG_INFO("Invalid package!");
            return;
        }
        ENGINE_LOG_INFO("Added mod prototype {}", package->name);
        manager->m_package_prototype_list[package->name] = *package;
        all_mods[package->name] = false;
    };

    ENGINE_LOG_INFO("Loading potential mods");

    // Load core
    mod_load(LoadModPrototype((data_path / "core").string()));

    // Enable core by default
    all_mods["core"] = true;

    // Load other packages
    std::filesystem::path save_path(cqsp::common::util::GetCqspAppDataPath());
    std::filesystem::path mods_folder = save_path / "mods";
    if (!std::filesystem::exists(mods_folder)) {
        std::filesystem::create_directories(mods_folder);
    }

    // Load all the mods
    std::filesystem::directory_iterator mods_folder_iterator(mods_folder);
    for (auto mod_element : mods_folder_iterator) {
        mod_load(LoadModPrototype(mod_element.path().string()));
    }
    ENGINE_LOG_INFO("Loaded mods folder");

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

std::string AssetLoader::GetModFilePath() {
    return (std::filesystem::path(cqsp::common::util::GetCqspAppDataPath()) / "mod.hjson").string();
}

std::optional<PackagePrototype> AssetLoader::LoadModPrototype(const std::string& path_string) {
    ZoneScoped;
    // Load the info.hjson
    std::filesystem::path package_path(path_string);
    IVirtualFileSystem* vfs = GetVfs(path_string);

    if (!vfs->IsFile("info.hjson")) {
        ENGINE_LOG_INFO("Mod prototype unable to be loaded from {}", path_string);
        return std::nullopt;
    }
    // Read mod info file.
    Hjson::Value mod_info = Hjson::Unmarshal(ReadAllFromVFileToString(vfs->Open("info.hjson").get()));

    // Get the info from
    PackagePrototype prototype;
    try {
        prototype.name = mod_info["name"].to_string();
        prototype.version = mod_info["version"].to_string();
        prototype.title = mod_info["title"].to_string();
        prototype.author = mod_info["author"].to_string();
        prototype.path = package_path.string();
    } catch (Hjson::index_out_of_bounds& ex) {
        // Don't load the mod, because prototype is invalid
        ENGINE_LOG_INFO("Hjson::index_out_of_bounds: {}", ex.what());
        return std::nullopt;
    }

    // Free memory
    delete vfs;
    return std::optional<PackagePrototype>(prototype);
}

std::unique_ptr<Package> AssetLoader::LoadPackage(std::string path) {
    ZoneScoped;
    // Load into filesystem
    // Load the assets of a package specified by a path
    // First load info.hjson, the info path of the file.
    std::filesystem::path package_path(path);

    // Mount the path
    IVirtualFileSystem* vfs = GetVfs(package_path.string());

    // Mount package path
    ENGINE_LOG_INFO("Loading package {}", package_path.string());
    // Read info.hjson from package
    auto info_file = vfs->Open("info.hjson");
    if (info_file == nullptr) {
        ENGINE_LOG_ERROR("Failed to load package {}", package_path.string());
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

    ENGINE_LOG_INFO("Mounted package {}", package->name);

    // Load dependencies
    // Now load the 'important' folders
    std::filesystem::path script_path(package_path / "scripts");
    // Check if files exist
    // Load scripts
    if (mounter.IsDirectory(mount_point, "scripts") && mounter.IsFile(mount_point, "scripts/base.lua")) {
        // Load base.lua for the base folder
        package->assets["base"] = LoadText(&mounter, mount_point + "/scripts/base.lua", "base", Hjson::Value());
        package->assets["scripts"] = LoadScriptDirectory(&mounter, mount_point + "/scripts", Hjson::Value());
        ENGINE_LOG_INFO("Loaded scripts");
    } else {
        ENGINE_LOG_INFO("No script file for package {}", package->name);
    }

    // Load a few other hjson folders.
    // So the folders we have to keep track off are the goods and recipes
    HjsonPrototypeDirectory(*package, fmt::format("{}/data/goods", mount_point), "goods");
    HjsonPrototypeDirectory(*package, fmt::format("{}/data/recipes", mount_point), "recipes");
    HjsonPrototypeDirectory(*package, fmt::format("{}/data/names", mount_point), "names");

    ENGINE_LOG_INFO("Loaded prototype directories");

    // Then load all the other assets
    // Load resource.hjsons
    LoadResources(*package, package->name);
    ENGINE_LOG_INFO("Package {} has {} assets", package->name, package->assets.size());
    return package;
}

std::unique_ptr<Asset> AssetLoader::LoadAsset(const AssetType& type, const std::string& path, const std::string& key,
                                              const Hjson::Value& hints) {
    // Load asset
    if (loading_functions.find(type) == loading_functions.end()) {
        ENGINE_LOG_WARN("{} asset loading not supported yet", ToString(type));
        return nullptr;
    }
    // Ensure path exists
    if (!mounter.Exists(path)) {
        ENGINE_LOG_WARN("{} at {} does not exist, errors may ensue", key, path);
    }
    return std::move(loading_functions[type](&mounter, path, key, hints));
}
void AssetLoader::PlaceAsset(Package& package, const AssetType& type, const std::string& path, const std::string& key,
                             const Hjson::Value& hints) {
    ZoneScoped;
    ENGINE_LOG_TRACE("Loading asset {}", path);
    auto asset = LoadAsset(type, path, key, hints);
    if (asset == nullptr) {
        ENGINE_LOG_WARN("Asset {} was not loaded properly", key);
        return;
    }
    asset->path = path;
    package.assets[key] = std::move(asset);
}

void AssetLoader::BuildNextAsset() {
    ZoneScoped;
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
        case PrototypeType::TEXTURE: {
            ImagePrototype* texture_prototype = dynamic_cast<ImagePrototype*>(temp.prototype);
            Texture* asset = dynamic_cast<Texture*>(texture_prototype->asset);
            asset::CreateTexture(*asset, texture_prototype->data, texture_prototype->width, texture_prototype->height,
                                 texture_prototype->components, texture_prototype->options);

            stbi_image_free(texture_prototype->data);
            break;
        }
        case PrototypeType::SHADER: {
            ShaderPrototype* shader = dynamic_cast<ShaderPrototype*>(temp.prototype);
            Shader* asset = dynamic_cast<Shader*>(shader->asset);
            try {
                int shaderId = asset::LoadShaderData(shader->data, shader->type);
                asset->id = shaderId;
            } catch (std::runtime_error& error) {
                ENGINE_LOG_WARN("Exception in loading shader {}: {}", shader->key, error.what());
            }
            break;
        }
        case PrototypeType::FONT: {
            FontPrototype* prototype = dynamic_cast<FontPrototype*>(temp.prototype);
            Font* asset = dynamic_cast<Font*>(prototype->asset);

            asset::LoadFontData(*asset, prototype->fontBuffer.data(), prototype->size);
        } break;
        case PrototypeType::CUBEMAP: {
            CubemapPrototype* prototype = dynamic_cast<CubemapPrototype*>(temp.prototype);
            Texture* asset = dynamic_cast<Texture*>(prototype->asset);
            asset::LoadCubemapData(*asset, prototype->data, prototype->width, prototype->height, prototype->components,
                                   prototype->options);
        } break;
    }

    // Free memory
    delete temp.prototype;
}

std::unique_ptr<Asset> AssetLoader::LoadText(VirtualMounter* mount, const std::string& path, const std::string& key,
                                             const Hjson::Value& hints) {
    ZoneScoped;
    if (!mount->IsFile(path)) {
        return nullptr;
    }
    std::unique_ptr<cqspa::TextAsset> asset = std::make_unique<cqspa::TextAsset>();
    auto file = mount->Open(path.c_str());
    int size = file->Size();
    asset->data = ReadAllFromVFileToString(file.get());
    return std::move(asset);
}

std::unique_ptr<Asset> AssetLoader::LoadTextDirectory(VirtualMounter* mount, const std::string& path,
                                                      const std::string& key, const Hjson::Value& hints) {
    ZoneScoped;
    if (!mount->IsDirectory(path)) {
        return nullptr;
    }
    std::unique_ptr<cqspa::TextDirectoryAsset> asset = std::make_unique<cqspa::TextDirectoryAsset>();
    auto dir = mount->OpenDirectory(path.c_str());
    int size = dir->GetSize();
    for (int i = 0; i < size; i++) {
        auto file = dir->GetFile(i);
        std::string buffer = ReadAllFromVFileToString(file.get());
        PathedTextAsset asset_data;
        asset_data.data = buffer;
        asset_data.path = path;
    }
    return std::move(asset);
}

std::unique_ptr<Asset> AssetLoader::LoadTexture(VirtualMounter* mount, const std::string& path, const std::string& key,
                                                const Hjson::Value& hints) {
    ZoneScoped;
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
    auto buffer = ReadAllFromVFile(file.get());
    prototype->data = stbi_load_from_memory(buffer.data(), file_size, &prototype->width, &prototype->height,
                                            &prototype->components, 0);
    if (prototype->data) {
        QueueHolder holder(prototype);

        m_asset_queue.push(holder);
    } else {
        ENGINE_LOG_ERROR("Failed to load image {}", key);
        delete prototype;
        return nullptr;
    }
    return std::move(texture);
}

std::unique_ptr<cqsp::asset::Asset> AssetLoader::LoadBinaryAsset(cqsp::asset::VirtualMounter* mount,
                                                                 const std::string& path, const std::string& key,
                                                                 const Hjson::Value& hints) {
    std::unique_ptr<BinaryAsset> asset = std::make_unique<BinaryAsset>();
    auto file = mount->Open(path);
    asset->data = ReadAllFromVFile(file.get());
    return asset;
}

std::unique_ptr<Asset> AssetLoader::LoadHjson(VirtualMounter* mount, const std::string& path, const std::string& key,
                                              const Hjson::Value& hints) {
    ZoneScoped;
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
                    // Append all the values in place
                    for (int k = 0; k < result.size(); k++) {
                        asset->data.push_back(result[k]);
                    }
                } else {
                    ENGINE_LOG_ERROR("Failed to load hjson file {}: it needs to be a array", file->Path());
                }
            } catch (Hjson::syntax_error& ex) {
                ENGINE_LOG_ERROR("Failed to load hjson file {}: {}", file->Path(), ex.what());
            }
        }
    } else {
        auto file = mount->Open(path.c_str());
        // Read the file
        try {
            asset->data = Hjson::Unmarshal(ReadAllFromVFileToString(file.get()), dec_opt);
        } catch (Hjson::syntax_error& ex) {
            ENGINE_LOG_ERROR("Failed to load hjson {}: {}", path, ex.what());
        }
    }
    return asset;
}

std::unique_ptr<Asset> AssetLoader::LoadShader(VirtualMounter* mount, const std::string& path, const std::string& key,
                                               const Hjson::Value& hints) {
    ZoneScoped;
    if (!mount->IsFile(path)) {
        return nullptr;
    }
    std::unique_ptr<Shader> shader = std::make_unique<Shader>();
    // Get shader type
    std::string type = hints["type"];
    int shader_type = -1;
    if (type == "frag") {
        shader_type = GL_FRAGMENT_SHADER;
        shader->shader_type = ShaderType::FRAG;
    } else if (type == "vert") {
        shader_type = GL_VERTEX_SHADER;
        shader->shader_type = ShaderType::VERT;
    } else if (type == "geom") {
        shader_type = GL_GEOMETRY_SHADER;
        shader->shader_type = ShaderType::GEOM;
    } else {
        // Abort, because this is a dud.
        ENGINE_LOG_WARN("Unsupport shader type: {}", key);
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

std::unique_ptr<Asset> AssetLoader::LoadFont(VirtualMounter* mount, const std::string& path, const std::string& key,
                                             const Hjson::Value& hints) {
    ZoneScoped;
    if (!mount->IsFile(path)) {
        return nullptr;
    }

    std::unique_ptr<Font> asset = std::make_unique<Font>();
    auto file = mount->Open(path);
    auto bytes = ReadAllFromVFile(file.get());

    FontPrototype* prototype = new FontPrototype();
    prototype->fontBuffer = std::move(bytes);
    prototype->size = file->Size();
    prototype->key = key;
    prototype->asset = asset.get();

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);

    return std::move(asset);
}

std::unique_ptr<Asset> AssetLoader::LoadAudio(VirtualMounter* mount, const std::string& path, const std::string& key,
                                              const Hjson::Value& hints) {
    ZoneScoped;
    // Load audio asset
    if (!mount->IsFile(path)) {
        return nullptr;
    }
    auto file = mount->Open(path);
    auto data = ReadAllFromVFile(file.get());
    auto asset = LoadOgg(data.data(), file->Size());
    return std::move(asset);
}

std::unique_ptr<Asset> AssetLoader::LoadCubemap(VirtualMounter* mount, const std::string& path, const std::string& key,
                                                const Hjson::Value& hints) {
    ZoneScoped;
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
        ENGINE_LOG_WARN("Cubemap {} does not have enough faces defined", key);
        return nullptr;
    }
    for (int i = 0; i < images_hjson.size(); i++) {
        std::string image_path = parent + "/" + images_hjson[i];
        if (!mount->IsFile(image_path)) {
            ENGINE_LOG_WARN("Cubemap {} has missing faces!", key);
            // Free memory
            for (auto texture : prototype->data) {
                // free the things
                stbi_image_free(texture);
            }
            return nullptr;
        }
        ZoneNamed(CubemapRead, true);
        auto file = mount->Open(image_path);
        auto file_data = ReadAllFromVFile(file.get());
        ZoneNamed(CubemapLoad, true);
        unsigned char* image_data = stbi_load_from_memory(file_data.data(), file->Size(), &prototype->width,
                                                          &prototype->height, &prototype->components, 0);
        prototype->data.push_back(image_data);
    }
    prototype->asset = asset.get();

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);
    return asset;
}

std::unique_ptr<ShaderDefinition> AssetLoader::LoadShaderDefinition(VirtualMounter* mount, const std::string& path,
                                                                    const std::string& key, const Hjson::Value& hints) {
    std::string parent = GetParentPath(path);
    auto root = mount->Open(path);
    std::string shader_def = ReadAllFromVFileToString(root.get());
    // Load shader def
    auto hjson = Hjson::Unmarshal(shader_def);
    // Load the files
    std::string vert_filename = parent + "/" + hjson["vert"];
    std::string frag_filename = parent + "/" + hjson["frag"];

    // Load the files
    if (!mount->Exists(vert_filename)) {
        ENGINE_LOG_INFO("Fragment file {} does not exist!", frag_filename);
        return nullptr;
    }
    auto vert_file = mount->Open(vert_filename);
    std::string vert_code = ReadAllFromVFileToString(vert_file.get());

    // Check if all the files exists
    if (!mount->Exists(frag_filename)) {
        ENGINE_LOG_INFO("Fragment file {} does not exist!", frag_filename);
        return nullptr;
    }
    auto frag_file = mount->Open(frag_filename);
    std::string frag_code = ReadAllFromVFileToString(frag_file.get());

    Hjson::Value uniforms = hjson["uniforms"];
    std::unique_ptr<ShaderDefinition> shader_def_ptr = std::make_unique<ShaderDefinition>();
    shader_def_ptr->uniforms = uniforms;
    shader_def_ptr->vert = vert_code;
    shader_def_ptr->frag = frag_code;

    // Load geometry file
    if (hjson["geom"].defined()) {
        std::string geom_filename = parent + "/" + hjson["geom"];

        // Then get the geometric matrix thing
        auto geom_file = mount->Open(geom_filename);
        std::string geom_code = ReadAllFromVFileToString(geom_file.get());
        shader_def_ptr->geometry = geom_code;
    }
    // Get uniforms, and then complain, I guess
    return shader_def_ptr;
}

std::unique_ptr<TextDirectoryAsset> AssetLoader::LoadScriptDirectory(VirtualMounter* mount, const std::string& path,
                                                                     const Hjson::Value& hints) {
    ZoneScoped;
    std::filesystem::path root(path);
    auto asset = std::make_unique<asset::TextDirectoryAsset>();
    if (!mount->IsDirectory(path)) {
        ENGINE_LOG_WARN("Script directory {} is not a script directory!", path);
        return nullptr;
    }
    auto directory = mount->OpenDirectory(path);
    for (int i = 0; i < directory->GetSize(); i++) {
        auto file = directory->GetFile(i);
        // Ensure that it's a lua file
        std::string file_path = file->Path();

        // Ignore non lua files
        if (file_path.substr(file_path.find_last_of(".") + 1) != "lua") {
            continue;
        }
        // Ignore base.lua file
        if (directory->GetFilename(i) == "base.lua") {
            continue;
        }
        // Then load the file
        std::string script = ReadAllFromVFileToString(file.get());

        std::string script_file_name = directory->GetFilename(i);

        // Replace the data path so that the name will be using dots
        // about file separators
        // So requiring a lua file (that is the purpose for this), will look like
        // require("test.abc")
        // to require a file called test/abc.lua

        // Remove file extension
        size_t lastdot = script_file_name.find_last_of(".");
        if (lastdot != std::string::npos) {
            script_file_name = script_file_name.substr(0, lastdot);
        }

        // Replace slashes with dots
        std::replace(script_file_name.begin(), script_file_name.end(), '/', '.');
        PathedTextAsset asset_data;
        asset_data.data = script;
        asset_data.path = script_file_name;
        asset->paths[script_file_name] = asset_data;
    }
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

void AssetLoader::LoadResources(Package& package, const std::string& package_mount_path) {
    ZoneScoped;
    // Load the package
    // Open the root directory
    auto directory = mounter.OpenDirectory(package_mount_path + "/");
    ENGINE_LOG_INFO("Loading {}", package_mount_path);
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
        } catch (Hjson::syntax_error& se) {
            ENGINE_LOG_WARN(se.what());
            // Then try again without the options
            dec_opt.duplicateKeyException = false;
            asset_value = Hjson::Unmarshal(asset_data, dec_opt);
        }

        max_loading += asset_value.size();
        LoadResourceHjsonFile(package, package_mount_path, resource_file->Path(), asset_value);
    }
}

void AssetLoader::LoadResourceHjsonFile(Package& package, const std::string& package_mount_path,
                                        const std::string& resource_file_path, const Hjson::Value& asset_value) {
    ZoneScoped;
    for (const auto [key, val] : asset_value) {
        ENGINE_LOG_TRACE("Loading asset {}", key);
        std::string type = val["type"];

        // Get the file
        std::string path;
        // Sometimes it's in the root directory of the directory, which would make this display 2 forward
        // slashes at a time, so this is to mitigate that.
        if (GetParentPath(resource_file_path).empty()) {
            path = package_mount_path + "/" + val["path"].to_string();
        } else {
            path = package_mount_path + "/" + GetParentPath(resource_file_path) + "/" + val["path"].to_string();
        }
        ENGINE_LOG_TRACE("Loading path {}", path);

        // Check if the file exists, just in case
        if (!mounter.Exists(path)) {
            ENGINE_LOG_WARN("Cannot find asset {} at {}", key, path);
            // Check if it's required
            if (!val["required"].empty() && val["required"]) {
                // Then required
                ENGINE_LOG_CRITICAL("Cannot find critical resource {}, exiting", key);
                missing_assets.push_back(key);
            }
            continue;
        }
        Hjson::Value hints;
        if (val["hints"].defined()) {
            hints = val["hints"];
        }
        PlaceAsset(package, FromString(type), path, std::string(key), hints);
        currentloading++;
    }
}
bool AssetLoader::HjsonPrototypeDirectory(Package& package, const std::string& path, const std::string& name) {
    ZoneScoped;
    if (!mounter.IsDirectory(path)) {
        return false;
    }
    package.assets[name] = LoadHjson(&mounter, path, name, Hjson::Value());
    return true;
}

IVirtualFileSystem* AssetLoader::GetVfs(const std::string& path) {
    // Return native filesystem for now.
    return new NativeFileSystem(path.c_str());
}
}  // namespace cqsp::asset
