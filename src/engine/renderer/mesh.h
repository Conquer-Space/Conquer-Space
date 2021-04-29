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

    // 1 is elements, 0 is arrays
    unsigned int buffer_type = 1;
};
}  // namespace engine
}  // namespace conquerspace
