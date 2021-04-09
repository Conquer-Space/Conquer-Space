/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>

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
};
}  // namespace engine
}  // namespace conquerspace
