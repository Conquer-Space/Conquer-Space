/*
* Copyright 2021 Conquer Space
*/
#include "engine/assetmanager.h"

#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <utility>
#include <algorithm>

#include "engine/renderer/shader.h"

conquerspace::asset::AssetManager::AssetManager() {}

conquerspace::asset::AssetLoader::AssetLoader() : m_asset_queue(16) {
    asset_type_map["none"] = AssetType::NONE;
    asset_type_map["texture"] = AssetType::TEXTURE;
    asset_type_map["shader"] = AssetType::SHADER;
    asset_type_map["hjson"] = AssetType::HJSON;
    asset_type_map["text"] = AssetType::TEXT;
    asset_type_map["model"] = AssetType::MODEL;
}

namespace cqspa = conquerspace::asset;

void conquerspace::asset::AssetLoader::LoadAssets(std::istream& stream) {
    Hjson::Value assets;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    stream >> Hjson::StreamDecoder(assets, decOpt);

    int size = static_cast<int>(assets.size());
    spdlog::info("Loading {} asset(s)", size);

    for (auto [key, val] : assets) {
        spdlog::trace("Loading {}", key);

        // Load from asset
        std::string asset_key = key;
        std::string type = val["type"];
        std::string path = "../data/core/" + val["path"];

        switch (asset_type_map[type]) {
            case AssetType::NONE:
            // Nothing to load
            break;
            case AssetType::TEXTURE:
            {
            LoadImage(asset_key, path, val["hints"]);
            break;
            }
            case AssetType::SHADER:
            {
            std::ifstream asset_stream(path);
            LoadShader(asset_key, asset_stream, val["hints"]);
            break;
            }
            case AssetType::HJSON:
            {
            std::ifstream asset_stream(path);
            if (asset_stream.good()) {
                manager->assets[key] = LoadHjson(asset_stream, val["hints"]);
            } else {
                spdlog::warn("Cannot find asset {}", key);
            }
            break;
            }
            case AssetType::TEXT:
            {
            std::ifstream asset_stream(path);
            if (asset_stream.good()) {
                manager->assets[key] = LoadText(asset_stream, val["hints"]);
            } else {
                spdlog::warn("Cannot find asset {}", key);
            }
            break;
            }
            case AssetType::MODEL:
            default:
                break;
        }
    }
}

void conquerspace::asset::AssetLoader::BuildNextAsset() {
    if (m_asset_queue.empty()) {
        return;
    }

    QueueHolder temp;
    m_asset_queue.pop(temp);

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
            spdlog::warn("Exception in loading shader {}: {}", shader->key, error.what());
        }
        break;
        }
    }

    // Free memory
    delete temp.prototype;
}

std::unique_ptr<cqspa::TextAsset> conquerspace::asset::AssetLoader::LoadText(
    std::istream& asset_stream, Hjson::Value hints) {
    std::unique_ptr<cqspa::TextAsset> asset =
        std::make_unique<cqspa::TextAsset>();

    // Load from string
    std::string asset_data{std::istreambuf_iterator<char>{asset_stream},
                            std::istreambuf_iterator<char>()};

    asset->data = asset_data;
    return asset;
}

std::unique_ptr<cqspa::HjsonAsset> cqspa::AssetLoader::LoadHjson(std::istream &asset_stream,
                                      Hjson::Value hints) {
    std::unique_ptr<cqspa::HjsonAsset> asset =
        std::make_unique<cqspa::HjsonAsset>();

    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    asset_stream >> Hjson::StreamDecoder(asset->data, decOpt);
    return asset;
}

void cqspa::AssetLoader::LoadImage(std::string& key,
                                                 std::string& filePath,
                                                 Hjson::Value hints) {
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

        if (!m_asset_queue.push(holder)) {
            spdlog::info("Failed to push image");
            delete prototype;
        }
    } else {
        spdlog::info("Failed to load {}", key);
        delete prototype;
    }
}

void cqspa::AssetLoader::LoadShader(std::string& key,
                                                  std::istream &asset_stream,
                                                  Hjson::Value hints) {
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

    if (!m_asset_queue.push(holder)) {
        spdlog::info("Failed to push image");
        delete(prototype);
    }
}
