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
#include "client/scenes/universe/interface/marketwindow.h"

#include <limits>

#include "GLFW/glfw3.h"
#include "client/scenes/universe/interface/markettable.h"
#include "client/scenes/universe/universescene.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/bodies.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"
#include "systooltips.h"

namespace cqsp::client::systems {
namespace cqspb = cqsp::common::components::bodies;
namespace cqspc = cqsp::common::components;

using cqsp::common::util::GetName;

void SysPlanetMarketInformation::Init() {}

void SysPlanetMarketInformation::DoUI(int delta_time) {
    // Get selected planet and display the market if it exists.
    if (!to_see) {
        return;
    }
    if (!GetUniverse().valid(selected_planet)) {
        return;
    }
    ImGui::Begin("Planetary Market");
    if (GetUniverse().any_of<common::components::Wallet>(selected_planet)) {
        auto& wallet = GetUniverse().get<cqspc::Wallet>(selected_planet);
        ImGui::TextFmt("GDP Contribution: {}", cqsp::util::LongToHumanString(wallet.GetGDPChange()));
        ImGui::TextFmt("Balance: {}", cqsp::util::LongToHumanString(wallet.GetBalance()));
        ImGui::TextFmt("Balance change: {}", cqsp::util::LongToHumanString(wallet.GetChange()));
    }
    MarketInformationTable(GetUniverse(), selected_planet);
    ImGui::End();
}

void SysPlanetMarketInformation::DoUpdate(int delta_time) {
    to_see = true;

    selected_planet = cqsp::scene::GetCurrentViewingPlanet(GetUniverse());
    entt::entity mouse_over = GetUniverse().view<MouseOverEntity>().front();
    if (!ImGui::GetIO().WantCaptureMouse && GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !cqsp::scene::IsGameHalted() && !GetApp().MouseDragged()) {
        to_see = true;

        if (GetUniverse().valid(selected_planet)) {
            SPDLOG_INFO("Switched entity: {}", GetUniverse().get<cqspc::Identifier>(selected_planet).identifier);
        } else {
            SPDLOG_INFO("Switched entity is not valid");
        }
    }
    if (!GetUniverse().valid(selected_planet) || !GetUniverse().all_of<cqspb::Body>(selected_planet)) {
        to_see = false;
    }
}
}  // namespace cqsp::client::systems
