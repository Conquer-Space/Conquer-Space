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
#include "marketwindow.h"

#include <limits>

#include "GLFW/glfw3.h"
#include "client/scenes/universe/universescene.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/bodies.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"
#include "systooltips.h"

namespace cqsp::client::systems {
namespace bodies = cqsp::common::components::bodies;
namespace components = cqsp::common::components;

using cqsp::common::util::GetName;
using cqsp::common::util::LongToHumanString;
using components::Market;
using entt::entity;
void MarketInformationTable(common::Universe& universe, const entity& market_entity) {
    if (!universe.any_of<Market>(market_entity)) {
        ImGui::TextFmt("Market is not a market");
        return;
    }
    // auto& center = GetUniverse().get<cqspc::MarketCenter>(marketentity);
    Market& market = universe.get<Market>(market_entity);
    ImGui::TextFmt("Has {} entities attached to it", market.participants.size());

    // Get resource stockpile
    if (!ImGui::BeginTable("marketinfotable", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        return;
    }
    ImGui::TableSetupColumn("Good");
    ImGui::TableSetupColumn("Price");
    ImGui::TableSetupColumn("Supply");
    ImGui::TableSetupColumn("Demand");
    ImGui::TableSetupColumn("S/D ratio");
    ImGui::TableSetupColumn("D/S ratio");
    ImGui::TableSetupColumn("Latent Demand");
    ImGui::TableSetupColumn("Input Ratio");
    ImGui::TableHeadersRow();
    auto goodsview = universe.view<components::Price>();

    for (entt::entity good_entity : goodsview) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (universe.any_of<components::CapitalGood>(good_entity)) {
            ImGui::TextFmtColored(ImColor(1.f, 1.f, 0.f), "{}", GetName(universe, good_entity));
        } else {
            ImGui::TextFmt("{}", GetName(universe, good_entity));
        }
        ImGui::TableSetColumnIndex(1);
        // Mark the cell as red if the thing is not valid
        ImGui::TextFmt("{}", market.price[good_entity]);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextFmt("{}",  LongToHumanString(market.previous_supply[good_entity]));
        ImGui::TableSetColumnIndex(3);
        ImGui::TextFmt("{}", LongToHumanString(market.previous_demand[good_entity]));
        ImGui::TableSetColumnIndex(4);
        double sd_ratio = market.sd_ratio[good_entity];
        if (sd_ratio == std::numeric_limits<double>::infinity()) {
            ImGui::TextFmt("inf");
        } else {
            ImGui::TextFmt("{}", sd_ratio);
        }
        ImGui::TableSetColumnIndex(5);
        ImGui::TextFmt("{}", market.ds_ratio[good_entity]);
        ImGui::TableSetColumnIndex(6);
        ImGui::TextFmt("{}", market.last_latent_demand[good_entity]);
        ImGui::TableSetColumnIndex(7);
        ImGui::TextFmt("{}", market[good_entity].inputratio);
    }
    ImGui::EndTable();
}

void SysPlanetMarketInformation::Init() {}

void SysPlanetMarketInformation::DoUI(int delta_time) {
    // Get selected planet and display the market if it exists.
    if (!to_see) {
        return;
    }
    if (!GetUniverse().valid(selected_planet)) {
        return;
    }
    ImGui::Begin("Market");
    MarketInformationTable(GetUniverse(), selected_planet);
    ImGui::End();
}

void SysPlanetMarketInformation::DoUpdate(int delta_time) {
    to_see = true;

    selected_planet = scene::GetCurrentViewingPlanet(GetUniverse());
    entity mouse_over = GetUniverse().view<MouseOverEntity>().front();
    if (!ImGui::GetIO().WantCaptureMouse && GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !scene::IsGameHalted() && !GetApp().MouseDragged()) {
        to_see = true;

        if (GetUniverse().valid(selected_planet)) {
            SPDLOG_INFO("Switched entity: {}", GetUniverse().get<components::Identifier>(selected_planet).identifier);
        } else {
            SPDLOG_INFO("Switched entity is not valid");
        }
    }
    if (!GetUniverse().valid(selected_planet) || !GetUniverse().all_of<components::bodies::Body>(selected_planet)) {
        to_see = false;
    }
}
}  // namespace cqsp::client::systems
