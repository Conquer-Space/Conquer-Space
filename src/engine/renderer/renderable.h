/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>
#include <string>

#include "engine/renderer/shader.h"
#include "engine/renderer/texture.h"
#include "engine/renderer/mesh.h"

namespace conquerspace {
namespace engine {
class Renderable {
 public:
    Mesh* mesh;
    conquerspace::asset::ShaderProgram* shaderProgram;
    std::vector<asset::Texture*> textures;
    glm::mat4 model;

    void SetTexture(const std::string& name, int id, asset::Texture* texture);

    int z = 0;
};
}  // namespace engine
}  // namespace conquerspace
