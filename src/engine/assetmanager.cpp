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
#include <regex>

#include <filesystem>

#include "engine/renderer/shader.h"
#include "engine/renderer/text.h"
#include "engine/audio/alaudioasset.h"

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

cqsp::asset::AssetManager::AssetManager() {}

cqsp::asset::ShaderProgram* cqsp::asset::AssetManager::CreateShaderProgram(
    const std::string& vert, const std::string& frag) {
    return new ShaderProgram(*GetAsset<cqsp::asset::Shader>(vert.c_str()),
                                    *GetAsset<cqsp::asset::Shader>(frag.c_str()));
}

void cqsp::asset::AssetManager::ClearAssets() {
    for(auto a = assets.begin(); a != assets.end(); a++) {
        a->second.reset();
    }
    assets.clear();
}

cqsp::asset::AssetLoader::AssetLoader() : m_asset_queue() {
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

namespace cqspa = cqsp::asset;

void cqsp::asset::AssetLoader::LoadAssets(std::istream& stream) {
    // Load enabled mods
    // Load core
    const char sep = std::filesystem::path::preferred_separator;
    std::filesystem::recursive_directory_iterator it(std::filesystem::path(fmt::format("..{0}data{0}core", sep)));

    for (auto a : it) {
        if (a.path().filename() == "resource.hjson") {
            // Load the particular asset folder
            // Open the file
            std::ifstream asset(a.path());
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

            for (const auto [key, val] : asset_value) {
                SPDLOG_TRACE("Loading {}", key);

                // Load asset
                std::string type = val["type"];
                std::string path = a.path().parent_path().string() + sep + val["path"];

                if (!std::filesystem::exists(path)) {
                    SPDLOG_WARN("Cannot find asset {}", key);
                    continue;
                }
                // Put in core namespace, I guess
                LoadAsset(type, path, std::string("core:" + key), val["hints"]);
            }
        }
    }
}

void cqsp::asset::AssetLoader::LoadAsset(const std::string& type,
                                                    const std::string& path,
                                                    const std::string& key,
                                                    const Hjson::Value &hints) {
    switch (asset_type_map[type]) {
        case AssetType::TEXTURE:
        {
        LoadImage(key, path, hints);
        break;
        }
        case AssetType::SHADER:
        {
        std::ifstream asset_stream(path, std::ios::binary);
        LoadShader(key, asset_stream, hints);
        break;
        }
        case AssetType::HJSON:
        {
        manager->AddAsset(key, LoadHjson(path, hints));
        break;
        }
        case AssetType::TEXT:
        {
        std::ifstream asset_stream(path);
        manager->AddAsset(key, LoadText(asset_stream, hints));
        break;
        }
        case AssetType::TEXT_ARRAY:
        {
        manager->AddAsset(key, LoadTextDirectory(path, hints));;
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
        manager->AddAsset(key, cqsp::asset::LoadOgg(asset_stream));
        break;
        }
        break;
        case AssetType::NONE:
        default:
        break;
    }
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
        std::unique_ptr<Texture> textureAsset = std::make_unique<Texture>();

        asset::LoadTexture(*textureAsset, texture_prototype->data,
            texture_prototype->components,
            texture_prototype->width,
            texture_prototype->height,
            texture_prototype->options);
        manager->AddAsset(key, std::move(textureAsset));
        break;
        }
        case PrototypeType::SHADER:
        {
        ShaderPrototype* shader = dynamic_cast<ShaderPrototype*>(temp.prototype);

        try  {
            int shaderId = asset::LoadShader(shader->data, shader->type);
            std::unique_ptr<Shader> shaderAsset = std::make_unique<Shader>();
            shaderAsset->id = shaderId;
            manager->AddAsset(key, std::move(shaderAsset));
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
            manager->AddAsset(key, std::move(fontAsset));
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

            manager->AddAsset(key, std::move(textureAsset));
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
        decOpt.comments = false;
        asset_stream >> Hjson::StreamDecoder(asset->data, decOpt);
    }
    return asset;
}

std::unique_ptr<cqsp::asset::TextDirectoryAsset>
cqsp::asset::AssetLoader::LoadTextDirectory(const std::string& path, const Hjson::Value& hints) {
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

void cqsp::asset::AssetLoader::LoadHjsonDir(const std::string& path, Hjson::Value& value,
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
    prototype->key = key;

    // Load image
    Hjson::Value pixellated = hints["magfilter"];

    if (pixellated.defined() && pixellated.type() == Hjson::Type::Bool) {
        // Then it's good
        prototype->options.mag_filter = static_cast<bool>(pixellated);
    } else {
        // Then loading it was a mistake (jk), then it's linear.
        prototype->options.mag_filter = false;
    }
    
    std::ifstream input(filePath, std::ios::binary);
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    char* data = new char[size];
    input.read(data, size);
    unsigned char* d = (unsigned char *) data;
    prototype->data = stbi_load_from_memory(d, size, &prototype->width, &prototype->height,
                           &prototype->components, 0);

    if (prototype->data) {
        QueueHolder holder(prototype);

        m_asset_queue.push(holder);
    } else {
        SPDLOG_INFO("Failed to load image {}", key);
        delete prototype;
    }
}

void cqspa::AssetLoader::LoadShader(const std::string& key, std::istream& asset_stream,
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
    prototype->hints = hints;
    std::istreambuf_iterator<char> eos;
    std::string s(std::istreambuf_iterator<char>(asset_stream), eos);
    prototype->data = s;

    QueueHolder holder(prototype);
    m_asset_queue.push(holder);
}

void cqsp::asset::AssetLoader::LoadFont(const std::string& key, std::istream& asset_stream,
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

void cqsp::asset::AssetLoader::LoadCubemap(const std::string& key,
                                                    const std::string &path,
                                                    std::istream &asset_stream,
                                                    const Hjson::Value& hints) {
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
