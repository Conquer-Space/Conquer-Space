/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>

namespace conquerspace {
namespace engine {

class Mesh {
 public:
    Mesh();

    ~Mesh();
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    unsigned int indicies;

    unsigned int RenderType;
};
}  // namespace engine
}  // namespace conquerspace
