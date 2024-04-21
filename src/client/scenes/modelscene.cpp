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
#include "client/scenes/modelscene.h"

#include <glm/gtc/matrix_transform.hpp>

#include "engine/graphics/model.h"
#include "engine/graphics/primitives/cube.h"

using cqsp::scene::ModelScene;

void ModelScene::Init() {
    // Load the model and shaders
    model = GetApp().GetAssetManager().GetAsset<asset::Model>("core:iss");
    // Load shader
    mesh = engine::primitive::MakeCube();
    shader = GetApp().GetAssetManager().GetAsset<asset::ShaderDefinition>("core:model_shader")->MakeShader();
}

void ModelScene::Update(float deltaTime) {}

void ModelScene::Ui(float deltaTime) {}

void ModelScene::Render(float deltaTime) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    // Render models
    glm::mat4 projection = glm::perspective(
        glm::radians(70.f), (float)GetApp().GetWindowWidth() / (float)GetApp().GetWindowHeight(), 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    shader->UseProgram();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    // render the loaded model
    glm::mat4 model_transform = glm::mat4(1.0f);
    model_transform = glm::translate(
        model_transform, glm::vec3(0.0f, 0.0f, 0.0f));  // translate it down so it's at the center of the scene
    model_transform =
        glm::scale(model_transform, glm::vec3(4.0f, 4.0f, 4.0f));  // it's a bit too big for our scene, so scale it down
    model_transform = glm::rotate(model_transform, (float)GetApp().GetTime(), glm::vec3(0, 1.f, 0.f));
    shader->setMat4("model", model_transform);
    //mesh->Draw();
    for (auto& model_mesh : model->meshes) {
        // Set the texture of the model mesh
        // Set the material
        // ISS just has a base diffuse color
        auto& material = model->materials[model_mesh->material];
        /*
            glActiveTexture(GL_TEXTURE0);
            int id = model->materials[model_mesh->material].diffuse.front()->id;
            glBindTexture(GL_TEXTURE_2D, id);
        */
        shader->setVec3("diffuse", material.base_diffuse);
        model_mesh->Draw();
    }
}
