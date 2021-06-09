/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include "client/systems/sysstarsystemrenderer.h"
#include "client/systems/sysplanetviewer.h"
#include "common/components/bodies.h"
#include "common/components/organizations.h"
#include "engine/scene.h"
#include "engine/application.h"
#include "engine/renderer/renderable.h"
#include "engine/renderer/renderer.h"
#include "common/simulation.h"

namespace conquerspace {
namespace scene {
class UniverseScene : public conquerspace::engine::Scene {
 public:
    explicit UniverseScene(conquerspace::engine::Application& app);
    ~UniverseScene() {
        delete system_renderer;
        delete simulation;
        delete planet_information;
    }

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

 private:
    conquerspace::engine::Renderable sphere;
    conquerspace::engine::Renderable sky;
    conquerspace::engine::Renderable planetDisp;
    conquerspace::engine::Renderable sun;

    float x = 0, y = 0;

    double previous_mouseX;
    double previous_mouseY;

    entt::entity player;
    entt::entity selected_planet = entt::null;

    conquerspace::components::Civilization* player_civ;
    conquerspace::components::bodies::StarSystem* star_system;

    conquerspace::client::systems::SysStarSystemRenderer* system_renderer;
    conquerspace::client::systems::SysPlanetInformation* planet_information;

    conquerspace::systems::simulation::Simulation* simulation;

    bool to_show_planet_window = false;

    double last_tick = 0;
};
}  // namespace scene
}  // namespace conquerspace
