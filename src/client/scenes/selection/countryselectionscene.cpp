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
#include "client/scenes/selection/countryselectionscene.h"

#include "client/components/clientctx.h"
#include "client/components/rightclick.h"

namespace cqsp::client::scene {

CountrySelectionScene::CountrySelectionScene(engine::Application& app,
                                             std::unique_ptr<systems::SysStarSystemRenderer> renderer)
    : ClientScene(app), system_renderer(std::move(renderer)) {}

CountrySelectionScene::~CountrySelectionScene() {}

void CountrySelectionScene::Init() {
    using core::systems::simulation::Simulation;
    simulation = std::make_unique<Simulation>(dynamic_cast<ConquerSpace*>(GetApp().GetGame())->GetGame());
    simulation->Init();
    // Init simulation tick
    simulation->tick();
}

void CountrySelectionScene::Update(float deltaTime) { system_renderer->Update(deltaTime); }

void CountrySelectionScene::Ui(float deltaTime) { system_renderer->DoUI(deltaTime); }

void CountrySelectionScene::Render(float deltaTime) {
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render(deltaTime);
}

}  // namespace cqsp::client::scene
