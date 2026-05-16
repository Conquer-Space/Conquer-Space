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
#include "client/scenes/selection/countryselectionmenu.h"
#include "client/scenes/universe/universescene.h"
#include "core/components/player.h"
#include "core/scripting/functionreg.h"

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

    AddRmlUiSystem<systems::rmlui::CountrySelectionMenu>();
    // Init our lua functions
    InitializeLuaFunctions();
}

void CountrySelectionScene::Update(float deltaTime) {
    for (auto& ui : documents) {
        ui->Update(deltaTime);
    }
    system_renderer->Update(deltaTime);
    entt::entity new_selected_country = system_renderer->GetSelectedCountry();

    if (selected_country != new_selected_country) {
        // Fire new event
        selected_country = new_selected_country;
        for (auto& ui : documents) {
            ui->DispatchEvent("provinceselect", {});
        }
    }
}

void CountrySelectionScene::Ui(float deltaTime) { system_renderer->DoUI(deltaTime); }

void CountrySelectionScene::Render(float deltaTime) {
    glEnable(GL_MULTISAMPLE);
    system_renderer->Render(deltaTime);
}

void CountrySelectionScene::StartGame() {
    // Set the next scene and move everything as well
    auto player = GetUniverse().countries["usa"];
    //universe.emplace<components::Civilization>(player);
    GetUniverse().emplace<core::components::Player>(GetUniverse().countries["usa"]);
    GetApp().SetScene<UniverseScene>(std::move(system_renderer), std::move(simulation));
}

void CountrySelectionScene::InitializeLuaFunctions() {
    auto& script_engine = GetApp().GetSolState();
    auto& universe = GetUniverse();
    CREATE_NAMESPACE(selection);

    REGISTER_FUNCTION("start_game", [&]() { StartGame(); });
    REGISTER_FUNCTION("get_selected_country", [&]() { return selected_country; });
}
}  // namespace cqsp::client::scene
