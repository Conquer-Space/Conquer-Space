/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "engine/scene.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/primitives/pane.h"
#include "engine/renderer/text.h"

namespace conquerspace {
namespace scene {
class MainMenuScene : public conquerspace::engine::Scene{
 public:
    explicit MainMenuScene(conquerspace::engine::Application& app);
    ~MainMenuScene() { }

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    conquerspace::asset::Texture* m_texture;
    conquerspace::asset::TextAsset* m_credits;
    bool m_credits_window = false;
    bool m_save_game_window = false;
    bool m_new_game_window = false;
    bool m_options_window = false;

    float ratio, ratio2;
    float width, height;

    conquerspace::engine::BasicRenderer renderer;
};
}  // namespace scene
}  // namespace conquerspace
