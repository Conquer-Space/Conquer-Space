/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "client/scenes/scene.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "client/systems/sysgui.h"
#include "common/components/bodies.h"
#include "common/components/organizations.h"
#include "common/simulation.h"
#include "engine/application.h"
#include "engine/graphics/renderable.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/renderer2d.h"

namespace cqsp::client::scene {
class UniverseScene : public ClientScene {
 public:
    explicit UniverseScene(engine::Application& app);
    ~UniverseScene() {
        // Delete ui
        simulation.reset();
        for (auto it = user_interfaces.begin(); it != user_interfaces.end(); it++) {
            it->reset();
        }
        for (auto& it : documents) {
            it.reset();
        }
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

    template <class T>
    void AddRmlUiSystem() {
        auto ui = std::make_unique<T>(GetApp());
        ui->OpenDocument();
        documents.push_back(std::move(ui));
    }

 private:
    /// <summary>
    /// Does the screenshot interface.
    /// </summary>
    void DoScreenshot();

    engine::Renderable sphere;
    engine::Renderable sky;
    engine::Renderable planetDisp;
    engine::Renderable sun;

    float x = 0, y = 0;

    double previous_mouseX;
    double previous_mouseY;

    entt::entity player;
    entt::entity selected_planet = entt::null;

    cqsp::client::systems::SysStarSystemRenderer* system_renderer;

    std::unique_ptr<cqsp::common::systems::simulation::Simulation> simulation;

    bool to_show_planet_window = false;

    // False is galaxy view, true is star system view
    bool view_mode = true;

    std::vector<std::unique_ptr<cqsp::client::systems::SysUserInterface>> user_interfaces;

    std::vector<std::unique_ptr<client::systems::SysRmlUiInterface>> documents;

    double last_tick = 0;

    void ToggleTick();

    bool interp = true;
};

void SeePlanet(common::Universe& universe, entt::entity ent);
entt::entity GetCurrentViewingPlanet(common::Universe&);
// Halts all other things
void SetGameHalted(bool b);
bool IsGameHalted();
}  // namespace cqsp::client::scene
