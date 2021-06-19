/*
* Copyright 2021 Conquer Space
*/
#include "engine/renderer/renderable.h"

void conquerspace::engine::Renderable::SetTexture(const std::string& name,
                                                  int id,
                                                  asset::Texture* texture) {
    shaderProgram->UseProgram();
    shaderProgram->setInt(name, id);

    textures.push_back(texture);
}

void conquerspace::engine::Renderable::SetMVP(glm::mat4& model, glm::mat4& view,
                                              glm::mat4& projection) {
    shaderProgram->UseProgram();
    shaderProgram->setMat4("model", model);
    shaderProgram->setMat4("view", view);
    shaderProgram->setMat4("projection", projection);
}
