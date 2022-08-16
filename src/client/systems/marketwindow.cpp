/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "client/systems/marketwindow.h"

#include <GLFW/glfw3.h>

#include <limits>

#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/economy.h"

#include "client/systems/views/starsystemview.h"
#include "client/scenes/universescene.h"
#include "client/systems/gui/systooltips.h"
#include "common/util/utilnumberdisplay.h"

namespace cqsp::client::systems {
namespace cqspb = cqsp::common::components::bodies;
namespace cqspc = cqsp::common::components;

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
    MarketInformationTooltipContent(selected_planet);
    ImGui::End();
}


void SysPlanetMarketInformation::MarketInformationTooltipContent(
    const entt::entity marketentity) {
    if (!GetUniverse().any_of<cqspc::Market>(marketentity)) {
        ImGui::TextFmt("Market is not a market");
        return;
    }
    // auto& center = GetUniverse().get<cqspc::MarketCenter>(marketentity);
    cqspc::Market& market = GetUniverse().get<cqspc::Market>(marketentity);
    ImGui::TextFmt("Has {} entities attached to it",
                   market.participants.size());

    // Get resource stockpile
    if (ImGui::BeginTable("marketinfotable", 6,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Price");
        ImGui::TableSetupColumn("Supply");
        ImGui::TableSetupColumn("Demand");
        ImGui::TableSetupColumn("S/D ratio");
        ImGui::TableSetupColumn("D/S ratio");
        ImGui::TableHeadersRow();
        auto goodsview = GetUniverse().view<cqspc::Price>();
        cqspc::MarketInformation lastmarket = market.history.back();
        for (entt::entity good_entity : goodsview) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt(
                "{}",
                GetUniverse().get<cqspc::Identifier>(good_entity).identifier);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", lastmarket.price[good_entity]);
            ImGui::TableSetColumnIndex(2);
            // auto& hist =
            // market.last_market_information[good_information.first];
            ImGui::TextFmt("{}", cqsp::util::LongToHumanString(
                                     lastmarket.supply[good_entity]));
            ImGui::TableSetColumnIndex(3);
            ImGui::TextFmt("{}", cqsp::util::LongToHumanString(
                                     lastmarket.demand[good_entity]));
            ImGui::TableSetColumnIndex(4);
            double sd_ratio = lastmarket.sd_ratio[good_entity];
            if (sd_ratio == std::numeric_limits<double>::infinity())
                ImGui::TextFmt("inf");
            else
                ImGui::TextFmt("{}", sd_ratio);
            ImGui::TableSetColumnIndex(5);
            ImGui::TextFmt("{}", lastmarket.ds_ratio[good_entity]);
            // Check if demand is 0, then supply is infinite, so
        }
        ImGui::EndTable();
    }
}

void SysPlanetMarketInformation::DoUpdate(int delta_time) {
    to_see = true;

    selected_planet = cqsp::scene::GetCurrentViewingPlanet(GetUniverse());
    entt::entity mouse_over = GetUniverse().view<MouseOverEntity>().front();
    if (!ImGui::GetIO().WantCaptureMouse &&
        GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !cqsp::scene::IsGameHalted() &&
        !GetApp().MouseDragged()) {
        to_see = true;

        if (GetUniverse().valid(selected_planet)) {
            SPDLOG_INFO("Switched entity: {}",
                        GetUniverse()
                            .get<cqspc::Identifier>(selected_planet)
                            .identifier);
        } else {
            SPDLOG_INFO("Switched entity is not valid");
        }
    }
    if (!GetUniverse().valid(selected_planet) ||
        !GetUniverse().all_of<cqspb::Body>(selected_planet)) {
        to_see = false;
    }
}
}  // namespace cqsp::client::systems