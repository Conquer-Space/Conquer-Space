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
#include "client/scenes/universe/interface/civilizationinfopanel.h"

#include <limits>

#include "client/components/clientctx.h"
#include "client/scenes/universe/interface/marketwindow.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "common/components/bodies.h"
#include "common/components/economy.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"

using cqsp::common::util::GetName;
using cqsp::common::util::LongToHumanString;
using cqsp::client::systems::CivilizationInfoPanel;

void CivilizationInfoPanel::Init() {}

void CivilizationInfoPanel::DoUI(int delta_time) {
    // Get the ui
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (to_display) {
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.2, ImGui::GetIO().DisplaySize.y * 0.4),
                                 ImGuiCond_Appearing);
    } else {
        ImGui::SetNextWindowSize(ImVec2(-1, -1));
    }
    ImGui::Begin("Civilization Window");
    if (ImGui::Button("Show/Hide Civilization")) {
        to_display = !to_display;
    }
    if (to_display) {
        CivInfoPanel();
    }
    ImGui::End();
}

void CivilizationInfoPanel::DoUpdate(int delta_time) {}

void CivilizationInfoPanel::CivInfoPanel() {
    // Get player
    entt::entity player = GetUniverse().view<common::components::Player>().front();
    if (player == entt::null) {
        return;
    }
    ImGui::TextFmt("{}", GetName(GetUniverse(), player));

    // Make hoverable
    gui::EntityTooltip(GetUniverse(), player);

    // If it has a capital city
    if (GetUniverse().any_of<common::components::Country>(player)) {
        entt::entity capital = GetUniverse().get<common::components::Country>(player).capital_city;
        if (capital != entt::null) {
            ImGui::TextFmt("Capital City: {}", GetName(GetUniverse(), capital));
        }
    }

    if (GetUniverse().any_of<common::components::Wallet>(player)) {
        auto& wallet = GetUniverse().get<common::components::Wallet>(player);
        ImGui::TextFmt("Reserves: {}", LongToHumanString(wallet.GetBalance()));
    }

    if (ImGui::BeginTabBar("civ_info_window")) {
        if (ImGui::BeginTabItem("City Information")) {
            // Collate all the owned stuff
            auto view = GetUniverse().view<common::components::Governed>();
            ImGui::Separator();
            ImGui::Text("Owned Cities");

            ImGui::BeginChild("ownedcitiespanel");
            for (auto entity : view) {
                if (GetUniverse().get<common::components::Governed>(entity).governor == player) {
                    ImGui::TextFmt("{}", GetName(GetUniverse(), entity));
                    gui::EntityTooltip(GetUniverse(), entity);
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Province Information")) {
            if (GetUniverse().any_of<common::components::CountryCityList>(player)) {
                for (entt::entity entity :
                     GetUniverse().get<common::components::CountryCityList>(player).province_list) {
                    bool selected = GetUniverse().view<ctx::SelectedProvince>().front() == entity;
                    if (ImGui::SelectableFmt("{}", &selected, GetName(GetUniverse(), entity))) {
                        entt::entity ent = GetUniverse().view<ctx::SelectedProvince>().front();
                        if (ent != entt::null) {
                            GetUniverse().remove<ctx::SelectedProvince>(ent);
                        }
                        // Set the entity
                        GetUniverse().emplace<ctx::SelectedProvince>(entity);
                    }
                    gui::EntityTooltip(GetUniverse(), entity);
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Market Information")) {
            MarketInformationTable(GetUniverse(), player);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Budget")) {
            BudgetInfoPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Markets")) {
            PlanetMarketInfoPanel();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void CivilizationInfoPanel::BudgetInfoPanel() {
    entt::entity player = GetUniverse().view<common::components::Player>().front();

    if (GetUniverse().any_of<common::components::Wallet>(player)) {
        auto& wallet = GetUniverse().get<common::components::Wallet>(player);
        ImGui::TextFmt("Reserves: {}", LongToHumanString(wallet.GetBalance()));
    } else {
        ImGui::TextFmt("No Wallet!");
    }

    // Then show the liabilities where they bankroll things
}

void CivilizationInfoPanel::PlanetMarketInfoPanel() {
    if (!ImGui::BeginTabBar("market_info_panel")) {
        return;
    }
    auto view = GetUniverse().view<common::components::Market, common::components::PlanetaryMarket>();
    for (entt::entity entity : view) {
        if (!ImGui::BeginTabItem(fmt::format("{}", GetName(GetUniverse(), entity)).c_str())) {
            continue;
        }
        ImGui::TextFmt("{}", GetName(GetUniverse(), entity));
        auto& market = GetUniverse().get<common::components::Market>(entity);
        client::systems::MarketInformationTable(GetUniverse(), entity);
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
