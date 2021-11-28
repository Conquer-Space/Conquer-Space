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
#include "common/util/paths.h"

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
    return count(asset) != 0;
}
bool cqsp::asset::Package::HasAsset(const std::string& asset) {
    return count(asset) != 0;
}

void cqsp::asset::Package::ClearAssets() {
    for (auto a = begin(); a != end(); a++) {
        a->second->Delete();
        a->second.reset();
    }
    clear();
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
    asset::LoadTexture(empty_texture, buffer, 2, 2, 3, f);
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
    for (auto it = potential_mods.begin(); it != potential_mods.end(); it++) {
        enabled_mods[it->second.name] = it->second.enabled;
    }
    // Write to file
    std::string mods_path = (std::filesystem::path(common::util::GetCqspSavePath())/"mod.hjson").string();
    Hjson::MarshalToFile(enabled_mods, mods_path);
    SPDLOG_INFO("Writing mods");
}

cqsp::asset::AssetLoader::AssetLoader() : m_asset_queue() {
}

namespace cqspa = cqsp::asset;

void cqsp::asset::AssetLoader::LoadAssets() {
    // Load enabled mods
    // Load core
    std::filesystem::path data_path(cqsp::common::util::GetCqspDataPath());
    std::filesystem::recursive_directory_iterator it(data_path);
    manager->packages["core"] = LoadPackage((data_path/"core").string());
    SPDLOG_INFO("Loaded core");

    // Load mods from the document folders
    // Find documents folder and load the information about the files

    // Keep track of all mods so that we can ensure that all are loaded
    Hjson::Value all_mods;

    // Some lambda things to keep things less cluttered and simpler
    auto mod_load = [&](const std::string &name) {
        all_mods[name] = false;
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

    // List files
    std::filesystem::directory_iterator mods_folder_iterator(mods_folder);
    for (auto mod_element : mods_folder_iterator) {
        mod_load(LoadModPrototype(mod_element.path().string()));
    }

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
        manager->potential_mods[it.first].enabled = static_cast<bool>(it.second);
    }

    // Enable mods, but let's think about that later
    for (auto it : manager->potential_mods) {
        // Get the thing
        manager->packages[it.first] = LoadPackage(it.second.path);
    }
}

std::string cqsp::asset::AssetLoader::GetModFilePath() {
    return (std::filesystem::path(cqsp::common::util::GetCqspSavePath())/"mod.hjson").string();
}

std::string cqsp::asset::AssetLoader::LoadModPrototype(const std::string& path_string) {
    std::filesystem::path path(path_string);
    // Load the info.hjson
    if (!std::filesystem::is_directory(path)) {
        return "";
    }
    if (!std::filesystem::exists(path/"info.hjson")) {
        return "";
    }
    Hjson::Value mod_info = Hjson::UnmarshalFromFile((path/"info.hjson").string());
    // Get the info of the file
    // Add core
    PackagePrototype prototype;
    try {
        prototype.name = mod_info["name"].to_string();
        prototype.version = mod_info["version"].to_string();
        prototype.title = mod_info["title"].to_string();
        prototype.author = mod_info["author"].to_string();
        prototype.path = path.string();
        manager->potential_mods[prototype.name] = prototype;
    } catch (Hjson::index_out_of_bounds &ex) {
        // Don't load the mod
        SPDLOG_INFO("Hjson::index_out_of_bounds: {}", ex.what());
    }
    return prototype.name;
}

void cqsp::asset::AssetLoader::LoadHjsonDirectory(Package& package, std::string path, std::string key) {
    Hjson::Value hints;
    std::unique_ptr<HjsonAsset> asset = std::make_unique<HjsonAsset>();
    // Verify that the directory exists
    if (std::filesystem::exists(std::filesystem::path(path)) &&
                std::filesystem::is_directory(std::filesystem::path(path))) {
        LoadHjsonDir(path, asset->data, hints);
        asset->original_path = path;
        package[key] = std::move(asset);
    }
}

std::unique_ptr<cqsp::asset::Package> cqsp::asset::AssetLoader::LoadPackage(std::string path) {
    // Load the assets of a package specified by a path
    // First load info.hjson, the info path of the file.
    std::filesystem::path package_path(path);
    SPDLOG_INFO("Loading package {}", package_path.string());
    Hjson::Value info = Hjson::UnmarshalFromFile((package_path/"info.hjson").string());
    // This will also serve as the namespace name, so no spaces, periods, semicolons please

    std::unique_ptr<Package> package = std::make_unique<Package>();
    package->name = info["name"].to_string();
    package->version = info["version"].to_string();
    package->title = info["title"].to_string();
    package->author = info["author"].to_string();

    // Load dependencies
    // Now load the 'important' folders
    std::filesystem::path script_path(package_path / "scripts");
    // Load scripts
    if (std::filesystem::exists(script_path) && std::filesystem::is_directory(script_path)) {
        // Load base.lua for the base folder
        std::string base_file_path = (script_path / "base.lua").string();
        auto base_stream = std::fstream(base_file_path);
        auto base = LoadText(base_stream, Hjson::Value());
        base->original_path = base_file_path;

        (*package)["base"] = std::move(base);
        (*package)["scripts"] = LoadScriptDirectory((package_path / "scripts").string(), Hjson::Value());
    }

    // Load a few other hjson folders.
    // So the folders we have to keep track off are the goods and recipes
    LoadHjsonDirectory(*package, (package_path / "data" / "goods").string(), "goods");
    LoadHjsonDirectory(*package, (package_path / "data" / "recipes").string(), "recipes");

    // Then load all the other assets
    // Load resource.hjsons
    LoadResources(*package, package_path.string());
    return package;
}

void cqsp::asset::AssetLoader::LoadAsset(Package& package, const std::string& type, const std::string& path,
                                        const std::string& key, const Hjson::Value& hints) {
    LoadAsset(package, FromString(type), path, key, hints);
}

void cqsp::asset::AssetLoader::LoadAsset(Package& package,
                                         cqsp::asset::AssetType type,
                                         const std::string& path,
                                         const std::string& key,
                                         const Hjson::Value& hints) {
    switch (type) {
        case AssetType::TEXTURE: {
            auto texture = LoadTexture(key, path, hints);
            texture->original_path = path;
            package[key] = std::move(texture);
            break;
        }
        case AssetType::SHADER: {
            std::ifstream asset_stream(path, std::ios::binary);
            auto shader = LoadShader(key, asset_stream, hints);
            shader->original_path = path;
            package[key] = std::move(shader);
            break;
        }
        case AssetType::HJSON: {
            // Check for errors
            auto p = LoadHjson(path, hints);
            if (p != nullptr) {
                p->original_path = path;
                package[key] = std::move(p);
            } else {
                SPDLOG_WARN("Failed to load asset {} due to hjson errors", key);
            }
            break;
        }
        case AssetType::TEXT: {
            std::ifstream asset_stream(path);
            auto text = LoadText(asset_stream, hints);
            text->original_path = path;
            package[key] = std::move(text);
            break;
        }
        case AssetType::TEXT_ARRAY: {
            break;
            auto text = LoadTextDirectory(path, hints);
            text->original_path = path;
            package[key] = std::move(text);
        }
        case AssetType::MODEL:
            break;
        case AssetType::CUBEMAP: {
            std::ifstream asset_stream(path);
            auto cubemap = LoadCubemap(key, path, asset_stream, hints);
            cubemap->original_path = path;
            package[key] = std::move(cubemap);
            break;
        }
        case AssetType::FONT: {
            std::ifstream asset_stream(path, std::ios::binary);
            auto font = LoadFont(key, asset_stream, hints);
            font->original_path = path;
            package[key] = std::move(font);
            break;
        }
        case AssetType::AUDIO: {
            std::ifstream asset_stream(path, std::ios::binary);
            auto audio = cqsp::asset::LoadOgg(asset_stream);
            audio->original_path = path;
            package[key] = std::move(audio);
            break;
        } break;
        case AssetType::NONE:
        default:
            // Don't load.
            break;
    }
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
        asset::LoadTexture(*asset,
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
            int shaderId = asset::LoadShader(shader->data, shader->type).id;
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

        asset::LoadCubemap(*asset, prototype->data,
            prototype->components,
            prototype->width,
            prototype->height,
            prototype->options);
        }
        break;
    }

    // Free memory
    delete temp.prototype;
}

std::unique_ptr<cqspa::TextAsset> cqsp::asset::AssetLoader::LoadText(std::istream& asset_stream,
                                                                    const Hjson::Value& hints) {
    std::unique_ptr<cqspa::TextAsset> asset = std::make_unique<cqspa::TextAsset>();

    // Load from string
    std::string asset_data{std::istreambuf_iterator<char>{asset_stream},
                            std::istreambuf_iterator<char>()};

    asset->data = asset_data;
    return asset;
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
            SPDLOG_INFO("Failed to load hjson {}: ", path, ex.what());
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

    // Set script file base path
    asset->original_path = path;
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
    asset->original_path = path;
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

void cqsp::asset::AssetLoader::LoadResources(Package& package, std::string path) {
    std::filesystem::recursive_directory_iterator it(path);
    for (auto resource_file : it) {
        if (resource_file.path().filename() == "resource.hjson") {
            // Load the particular asset folder
            // Open the file
            std::ifstream asset(resource_file.path());
            std::string asset_data(std::istreambuf_iterator<char>(asset), {});
            Hjson::Value asset_value;
            Hjson::DecoderOptions decOpt;
            decOpt.comments = false;
            decOpt.duplicateKeyException = true;

            // Try to load and check for duplicate options, sadly hjson doesn't provide good
            // ways to see which keys are duplicated, except by exception, so we'll have
            // to do this as a hack for now
            try {
                asset_value = Hjson::Unmarshal(asset_data, decOpt);
            } catch (Hjson::syntax_error &se) {
                SPDLOG_WARN(se.what());
                // Then try again without the options
                decOpt.duplicateKeyException = false;
                asset_value = Hjson::Unmarshal(asset_data, decOpt);
            }

            max_loading += asset_value.size();
            for (const auto [key, val] : asset_value) {
                SPDLOG_TRACE("Loading {}", key);

                // Load asset
                std::string type = val["type"];
                // absolute is so that it looks nice when displaying it
                std::string path = std::filesystem::absolute(resource_file.path().parent_path() /
                                    val["path"].to_string()).string();

                if (!std::filesystem::exists(path)) {
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
                LoadAsset(package, type, path, std::string(key), val["hints"]);
                currentloading++;
            }
        }
    }
}
