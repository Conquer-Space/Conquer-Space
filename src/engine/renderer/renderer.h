/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <memory>

#include <glm/glm.hpp>
#include "engine/renderer/renderable.h"

namespace conquerspace {
namespace engine {
void Draw(Renderable &);

typedef std::shared_ptr<Renderable> BasicRendererObject;

BasicRendererObject MakeRenderable();

class BasicRenderer {
 public:
    glm::mat4 projection = glm::mat4(1.0);
    glm::mat4 view = glm::mat4(1.0);
    std::vector<BasicRendererObject> renderables;
    void Draw();
};
}  // namespace engine
}  // namespace conquerspace
