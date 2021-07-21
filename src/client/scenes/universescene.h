/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "client/systems/sysstarsystemrenderer.h"
#include "client/systems/sysgui.h"
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
    }

    void Init();
    void Update(float deltaTime);
    void Ui(float deltaTime);
    void Render(float deltaTime);

    template <class T>
    void AddUISystem() {
        auto ui = std::make_unique<T>(GetApp());
        ui->Init();
        user_interfaces.push_back(std::move(ui));
    }

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

    conquerspace::common::components::Civilization* player_civ;
    conquerspace::common::components::bodies::StarSystem* star_system;

    conquerspace::client::systems::SysStarSystemRenderer* system_renderer;

    std::shared_ptr<conquerspace::common::systems::simulation::Simulation> simulation;

    bool to_show_planet_window = false;

    std::vector<std::unique_ptr<conquerspace::client::systems::SysUserInterface>> user_interfaces;
};

void SeePlanet(conquerspace::engine::Application&, entt::entity);
void SeeStarSystem(conquerspace::engine::Application&, entt::entity);
entt::entity GetCurrentViewingPlanet(conquerspace::engine::Application&);
// Halts all other things
void SetGameHalted(bool b);
bool IsGameHalted();
}  // namespace scene
}  // namespace conquerspace
