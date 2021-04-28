/*
* Copyright 2021 Conquer Space
*/
#include "client/scenes/universescene.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "engine/renderer/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/gui.h"

conquerspace::scene::UniverseScene::UniverseScene(
    conquerspace::engine::Application& app) : Scene(app) {}

void conquerspace::scene::UniverseScene::Init() {
    sphere.mesh = new conquerspace::engine::Mesh();
    conquerspace::primitive::ConstructSphereMesh(64, 64, *sphere.mesh);

    
    asset::ShaderProgram* program =
        GetApplication().GetAssetManager().CreateShaderProgram("objectvert",
                                                               "defaultfrag");

    asset::Texture* texture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earth");

    asset::Texture* earthNormalTexture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earthnormal");

    asset::Texture* earthRoughnessTexture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("earthroughness");

    sphere.shaderProgram = program;
    sphere.SetTexture("texture0", 0, texture);
    sphere.SetTexture("normalMap", 1, earthNormalTexture);
    sphere.SetTexture("roughnessMap", 2, earthRoughnessTexture);

    asset::ShaderProgram* skyboxProgram = GetApplication().GetAssetManager().CreateShaderProgram("objectvert",
                                                               "skyboxfrag");

    asset::Texture* skyTexture = GetApplication().GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("sky");

    sky.shaderProgram = skyboxProgram;
    sky.SetTexture("texture0", 0, skyTexture);

    sky.mesh = sphere.mesh;

    x = 0;
    y = -distance;

    previous_mouseX = GetApplication().GetMouseX();
    previous_mouseY = GetApplication().GetMouseY();
}

void conquerspace::scene::UniverseScene::Update(float deltaTime) {
    if (GetApplication().ButtonIsHeld(GLFW_KEY_LEFT)) {
        x += (deltaTime * 10);
    }
    if (GetApplication().ButtonIsHeld(GLFW_KEY_RIGHT)) {
        x -= (deltaTime * 10);
    }
    if (GetApplication().ButtonIsHeld(GLFW_KEY_UP)) {
        y += (deltaTime * 10);
    }
    if (GetApplication().ButtonIsHeld(GLFW_KEY_DOWN)) {
        y -= (deltaTime * 10);
    }

    if (scroll + GetApplication().GetScrollAmount() > 1.5 && scroll + GetApplication().GetScrollAmount() < 95) {
        scroll += GetApplication().GetScrollAmount();
    }

    // Now do things with it
    double deltaX = previous_mouseX - GetApplication().GetMouseX();
    double deltaY = previous_mouseY - GetApplication().GetMouseY();
    if(deltaY > 3.1415/2) {
        deltaY = 3.1415/2;
    }
    if(deltaY < -3.1415/2) {
        deltaY = -3.1415/2;
    }

    x += deltaX;
    y += deltaY;
    if (GetApplication().MouseButtonIsHeld(GLFW_MOUSE_BUTTON_LEFT)) {
        viewAngleX += deltaX/GetApplication().GetWindowWidth()*3.1415*4;
        viewAngleY -= deltaY/GetApplication().GetWindowHeight()*3.1415*4;
        if(glm::degrees(viewAngleY) > 89.f) {
            viewAngleY = glm::radians(89.f);
        }
        if(glm::degrees(viewAngleY) < -89.f) {
            viewAngleY = glm::radians(-89.f);
        }
    }

    previous_mouseX = GetApplication().GetMouseX();
    previous_mouseY = GetApplication().GetMouseY();
}

void conquerspace::scene::UniverseScene::Ui(float deltaTime) {
    ImGui::Begin("Window");
    ImGui::SliderFloat("Distance", &distance, 0, 1000);
    ImGui::SliderFloat("theta", &theta, 0, 3.14159*2);
    ImGui::Text("%f, %f", x, y);
    ImGui::Text("%f, %f", viewAngleX, viewAngleY);
    float vay = viewAngleY;
    ImGui::SliderFloat("viewAngleY", &vay, 0.f, 3.14159*2);
    viewAngleY = vay;
    ImGui::End();
}

void conquerspace::scene::UniverseScene::Render(float deltaTime) {
    glEnable(GL_DEPTH_TEST);

    sphere.shaderProgram->UseProgram();
    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(0, 0, -distance));
    model = glm::scale(model, glm::vec3(-1, 1, 1));

    sphere.shaderProgram->setVec3("lightDir",
                            glm::normalize(glm::vec3(-1.f, 0, -1.f) - glm::vec3(0, 0, -distance)));
    sphere.shaderProgram->setVec3("lightPos", glm::vec3(-1.f, 0, -1.f));
    sphere.shaderProgram->setVec3("lightColor", glm::vec3(50.f));
    sphere.shaderProgram->setVec3("objectColor", glm::vec3(1.f, 0, 0));
    sphere.shaderProgram->setMat4("model", model);
    glm::mat4 projection =
              glm::infinitePerspective(glm::radians(45.f),
                                static_cast<float>(GetApplication().GetWindowWidth()) /
                                static_cast<float>(GetApplication().GetWindowHeight()), 0.1f);

    sphere.shaderProgram->setMat4("projection", projection);
    glm::vec3 modelPosition = glm::vec3(model[3]);
    float camX = x;
    float camY = y;

    glm::vec3 camPos = glm::vec3(
                cos(viewAngleY) * sin(viewAngleX) * scroll,
                sin(viewAngleY) * scroll,
                cos(viewAngleY) * cos(viewAngleX) * scroll);

    float thingy = fmod(viewAngleY, 3.1415926535);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    camPos = camPos + modelPosition;
    sphere.shaderProgram->setVec3("viewPos", camPos);

    // For some reason, glm gets a but funky with a 0
    sphere.shaderProgram->setMat4("view", glm::lookAt(camPos, modelPosition, camUp));
    engine::Draw(sphere);

    sky.shaderProgram->UseProgram();
    // Follow camera, do nothing else lmao
    sky.shaderProgram->setMat4("view", glm::lookAt(camPos, modelPosition, camUp));
    sky.shaderProgram->setMat4("projection", projection);

    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(0, 0, -distance));
    model = glm::scale(model, glm::vec3(10, 10, 10));

    sky.shaderProgram->setMat4("model", model);
    glDepthFunc(GL_LEQUAL);
    engine::Draw(sky);
    glDepthFunc(GL_LEQUAL);
    glDepthFunc(GL_LESS);
}
