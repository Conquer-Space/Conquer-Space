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
#include "engine/assetmanager.h"

#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <utility>
#include <algorithm>

#include <filesystem>

#include "engine/renderer/shader.h"
#include "engine/renderer/text.h"
#include "engine/audio/alaudioasset.h"

conquerspace::asset::AssetManager::AssetManager() {}

conquerspace::asset::ShaderProgram* conquerspace::asset::AssetManager::CreateShaderProgram(
    const std::string& vert, const std::string& frag) {
    return new ShaderProgram(*GetAsset<conquerspace::asset::Shader>(vert.c_str()),
                                    *GetAsset<conquerspace::asset::Shader>(frag.c_str()));
}

void conquerspace::asset::AssetManager::ClearAssets() {
    for(auto a = assets.begin(); a != assets.end(); a++) {
        a->second.reset();
    }
    assets.clear();
}

conquerspace::asset::AssetLoader::AssetLoader() : m_asset_queue() {
    asset_type_map["none"] = AssetType::NONE;
    asset_type_map["texture"] = AssetType::TEXTURE;
    asset_type_map["shader"] = AssetType::SHADER;
    asset_type_map["hjson"] = AssetType::HJSON;
    asset_type_map["text"] = AssetType::TEXT;
    asset_type_map["model"] = AssetType::MODEL;
    asset_type_map["font"] = AssetType::FONT;
    asset_type_map["cubemap"] = AssetType::CUBEMAP;
    asset_type_map["directory"] = AssetType::TEXT_ARRAY;
    asset_type_map["audio"] = AssetType::AUDIO;
}

namespace cqspa = conquerspace::asset;

void conquerspace::asset::AssetLoader::LoadAssets(std::istream& stream) {
    Hjson::Value asset_value;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    stream >> Hjson::StreamDecoder(asset_value, decOpt);

    int size = static_cast<int>(asset_value.size());
    SPDLOG_INFO("Loading {} asset(s)", size);

    for (const auto [key, val] : asset_value) {
        SPDLOG_TRACE("Loading {}", key);

        // Load from asset
        std::string type = val["type"];
        std::string path = "../data/core/" + val["path"];

        if (!std::filesystem::exists(path)) {
            SPDLOG_WARN("Cannot find asset {}", key);
            continue;
        }
        LoadAsset(type, path, std::string(key), val["hints"]);
    }
}

void conquerspace::asset::AssetLoader::LoadAsset(const std::string& type,
                                                    const std::string& path,
                                                    const std::string& key,
                                                    const Hjson::Value &hints) {
switch (asset_type_map[type]) {
        case AssetType::NONE:
        // Nothing to load
        break;
        case AssetType::TEXTURE:
        {
        LoadImage(key, path, hints);
        break;
        }
        case AssetType::SHADER:
        {
        std::ifstream asset_stream(path);
        LoadShader(key, asset_stream, hints);
        break;
        }
        case AssetType::HJSON:
        {
            // Load a directory if it's a directory
            if (std::filesystem::is_directory(path)) {
                // Load and append to assets.
                Hjson::Value data;
                LoadHjsonDir(path, data, hints);
                std::unique_ptr<cqspa::HjsonAsset> asset =
                    std::make_unique<cqspa::HjsonAsset>();
                asset->data = data;
                manager->assets[key] = std::move(asset);
            } else {
                std::ifstream asset_stream(path);
                manager->assets[key] = LoadHjson(asset_stream, hints);
            }
        break;
        }
        case AssetType::TEXT:
        {
        std::ifstream asset_stream(path);
        manager->assets[key] = LoadText(asset_stream, hints);
        break;
        }
        case AssetType::TEXT_ARRAY:
        {
        manager->assets[key] = LoadTextDirectory(path, hints);
        break;
        }
        case AssetType::MODEL:
        break;
        case AssetType::CUBEMAP:
        {
        std::ifstream asset_stream(path);
        LoadCubemap(key, path, asset_stream, hints);
        break;
        }
        case AssetType::FONT:
        {
        std::ifstream asset_stream(path, std::ios::binary);
        LoadFont(key, asset_stream, hints);
        break;
        }
        case AssetType::AUDIO:
        {
        std::ifstream asset_stream(path, std::ios::binary);
        if (asset_stream.good()) {
            manager->assets[key] = conquerspace::asset::LoadOgg(asset_stream);
        } else {
            SPDLOG_WARN("Invalid ogg file {}", path);
        }
        break;
        }
        break;
        default:
        break;
    }
}

void conquerspace::asset::AssetLoader::BuildNextAsset() {
    if (m_asset_queue.size() == 0) {
        return;
    }

    auto value = m_asset_queue.pop();
    if (!value.has_value()) {
        return;
    }
    QueueHolder temp = *value;

    switch (temp.prototype->GetPrototypeType()) {
        case PrototypeType::TEXTURE:
        {
        ImagePrototype* texture_prototype = dynamic_cast<ImagePrototype*>(temp.prototype);
        std::unique_ptr<Texture> textureAsset = std::make_unique<Texture>();

        asset::LoadTexture(*textureAsset, texture_prototype->data,
            texture_prototype->components,
            texture_prototype->width,
            texture_prototype->height,
            texture_prototype->options);

        manager->assets[texture_prototype->key] = std::move(textureAsset);

        break;
        }
        case PrototypeType::SHADER:
        {
        ShaderPrototype* shader = dynamic_cast<ShaderPrototype*>(temp.prototype);

        try  {
            int shaderId = asset::LoadShader(shader->data, shader->type);
            std::unique_ptr<Shader> shaderAsset = std::make_unique<Shader>();
            shaderAsset->id = shaderId;
            manager->assets[shader->key] = std::move(shaderAsset);
        } catch (std::runtime_error &error) {
            SPDLOG_WARN("Exception in loading shader {}: {}", shader->key, error.what());
        }
        break;
        }
        case PrototypeType::FONT:
        {
            FontPrototype* prototype = dynamic_cast<FontPrototype*>(temp.prototype);
            std::unique_ptr<Font> fontAsset = std::make_unique<Font>();
            asset::LoadFont(*fontAsset.get(), prototype->fontBuffer, prototype->size);
            manager->assets[prototype->key] = std::move(fontAsset);
        }
        break;
        case PrototypeType::CUBEMAP:
        {
            CubemapPrototype* prototype = dynamic_cast<CubemapPrototype*>(temp.prototype);
            std::unique_ptr<Texture> textureAsset = std::make_unique<Texture>();

            asset::LoadCubemap(*textureAsset, prototype->data,
                prototype->components,
                prototype->width,
                prototype->height,
                prototype->options);

            manager->assets[prototype->key] = std::move(textureAsset);
        }
        break;
    }

    // Free memory
    delete temp.prototype;
}

std::unique_ptr<cqspa::TextAsset> conquerspace::asset::AssetLoader::LoadText(
    std::istream& asset_stream, const Hjson::Value& hints) {
    std::unique_ptr<cqspa::TextAsset> asset =
        std::make_unique<cqspa::TextAsset>();

    // Load from string
    std::string asset_data{std::istreambuf_iterator<char>{asset_stream},
                            std::istreambuf_iterator<char>()};

    asset->data = asset_data;
    return asset;
}

std::unique_ptr<cqspa::HjsonAsset> cqspa::AssetLoader::LoadHjson(std::istream &asset_stream,
                                    const Hjson::Value& hints) {
    std::unique_ptr<cqspa::HjsonAsset> asset =
        std::make_unique<cqspa::HjsonAsset>();

    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    asset_stream >> Hjson::StreamDecoder(asset->data, decOpt);
    return asset;
}

std::unique_ptr<conquerspace::asset::TextDirectoryAsset>
conquerspace::asset::AssetLoader::LoadTextDirectory(const std::string& path,
                                                    const Hjson::Value& hints) {
        std::filesystem::recursive_directory_iterator iterator(path);
        auto asset = std::make_unique<asset::TextDirectoryAsset>();
        for (auto& sub_path : iterator) {
            if (!sub_path.is_regular_file()) {
                continue;
            }
            std::ifstream asset_stream(sub_path.path().string());
            std::string asset_data{std::istreambuf_iterator<char>{asset_stream},
                            std::istreambuf_iterator<char>()};
            asset->data.push_back(asset_data);
        }
        return asset;
}

void cqspa::AssetLoader::LoadHjson(std::istream &asset_stream, Hjson::Value& value,
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

void conquerspace::asset::AssetLoader::LoadHjsonDir(const std::string& path, Hjson::Value& value,
                                                                            const Hjson::Value& hints) {
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path)) {
        // Loop through each hjson
        std::ifstream asset_stream(dirEntry.path().c_str());

        if (!asset_stream.good()) {
            continue;
        }
        LoadHjson(asset_stream, value, hints);
    }
}

void cqspa::AssetLoader::LoadImage(const std::string& key, const std::string& filePath,
                                                const Hjson::Value& hints) {
    ImagePrototype* prototype = new ImagePrototype();

    // Load image
    prototype->key = new char[key.size() + 1];
    std::copy(key.begin(), key.end(), prototype->key);
    prototype->key[key.size()] = '\0';

    Hjson::Value pixellated = hints["magfilter"];

    if (pixellated.defined() && pixellated.type() == Hjson::Type::Bool) {
        // Then it's good
        prototype->options.mag_filter = static_cast<bool>(pixellated);
    } else {
        // Then loading it was a mistake (jk), then it's linear.
        prototype->options.mag_filter = false;
    }
    prototype->data =
        stbi_load(filePath.c_str(),
            &prototype->width,
            &prototype->height,
            &prototype->components, 0);

    if (prototype->data) {
        QueueHolder holder(prototype);

        m_asset_queue.push(holder);
    } else {
        SPDLOG_INFO("Failed to load {}", key);
        delete prototype;
    }
}

void cqspa::AssetLoader::LoadShader(const std::string& key, std::istream &asset_stream,
                                                const Hjson::Value& hints) {
    // Get shader type
    std::string type = hints["type"];
    int shader_type = -1;
    if (type == "frag") {
        shader_type = GL_FRAGMENT_SHADER;
    } else if (type == "vert") {
        shader_type = GL_VERTEX_SHADER;
    } else {
        // Abort, because this is a dud.
        return;
    }

    ShaderPrototype* prototype = new ShaderPrototype();

    prototype->key = key;
    prototype->type = shader_type;
    std::istreambuf_iterator<char> eos;
    std::string s(std::istreambuf_iterator<char>(asset_stream), eos);
    prototype->data = s;

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);
}

void conquerspace::asset::AssetLoader::LoadFont(const std::string& key, std::istream& asset_stream,
                                                const Hjson::Value& hints) {
    asset_stream.seekg(0, std::ios::end);
    std::fstream::pos_type fontFileSize = asset_stream.tellg();
    asset_stream.seekg(0);
    unsigned char *fontBuffer = new unsigned char[fontFileSize];
    asset_stream.read(reinterpret_cast<char*>(fontBuffer), fontFileSize);

    FontPrototype* prototype = new FontPrototype();
    prototype->fontBuffer = fontBuffer;
    prototype->size = fontFileSize;
    prototype->key = key;

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);
}

void conquerspace::asset::AssetLoader::LoadCubemap(const std::string& key,
                                                    const std::string &path,
                                                    std::istream &asset_stream,
                                                    const Hjson::Value& hints) {
    // Read file, which will be hjson, and load those files too
    Hjson::Value images;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    asset_stream >> Hjson::StreamDecoder(images, decOpt);

    CubemapPrototype* prototype = new CubemapPrototype();

    prototype->key = new char[key.size() + 1];
    std::copy(key.begin(), key.end(), prototype->key);
    prototype->key[key.size()] = '\0';

    std::filesystem::path p(path);
    std::filesystem::path dir = p.parent_path();

    for (int i = 0; i < images.size(); i++) {
        std::string filePath = dir.string() + "/" + images[i];

        unsigned char* data =
            stbi_load(filePath.c_str(),
                &prototype->width,
                &prototype->height,
                &prototype->components, 0);

        prototype->data.push_back(data);
    }

    QueueHolder holder(prototype);

    m_asset_queue.push(holder);
}
