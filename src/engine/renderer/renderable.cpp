#include "engine/renderer/renderable.h"

void conquerspace::engine::Renderable::SetTexture(const std::string& name,
                                                  int id,
                                                  asset::Texture* texture) {
    shaderProgram->UseProgram();
    shaderProgram->setInt(name, id);

    textures.push_back(texture);
}
