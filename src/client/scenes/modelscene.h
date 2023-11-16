/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#pragma once

#include "client/scenes/scene.h"
#include "engine/graphics/mesh.h"
#include "engine/graphics/model.h"
#include "engine/graphics/shader.h"
#include "engine/renderer/framebuffer.h"

namespace cqsp {
namespace scene {

class ModelScene : public cqsp::client::Scene {
 public:
    explicit ModelScene(cqsp::engine::Application& app) : cqsp::client::Scene(app) {}
    ~ModelScene() = default;

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    asset::Model* model;
    asset::ShaderProgram_t shader;
    engine::Mesh_t mesh;
};
}  // namespace scene
}  // namespace cqsp
