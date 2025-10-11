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
#include "client/scenes/universe/interface/sysplanetmarketinformation.h"

#include <limits>

#include "GLFW/glfw3.h"
#include "client/scenes/universe/interface/markettable.h"
#include "client/scenes/universe/universescene.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/bodies.h"
#include "common/components/market.h"
#include "common/components/name.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"
#include "systooltips.h"

namespace cqsp::client::systems {
namespace components = common::components;
namespace bodies = components::bodies;

using common::util::GetName;

void SysPlanetMarketInformation::Init() {}

void SysPlanetMarketInformation::DoUI(int delta_time) {
    // Get selected planet and display the market if it exists.
    if (!to_see) {
        return;
    }
    if (!GetUniverse().valid(selected_planet)) {
        return;
    }
    ImGui::Begin(fmt::format("{} Planetary Market", common::util::GetName(GetUniverse(), selected_planet)).c_str());
    if (ImGui::BeginTabBar("Planetary Market Tab Bar", ImGuiTabBarFlags_None)) {
        LocalMarketInformation();
        InterplanetaryTradeInformation();
        InterplanetaryTradeAmounts();
        PerGoodDetails();
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void SysPlanetMarketInformation::DoUpdate(int delta_time) {
    to_see = true;

    selected_planet = scene::GetCurrentViewingPlanet(GetUniverse());
    entt::entity mouse_over = GetUniverse().view<MouseOverEntity>().front();
    if (!ImGui::GetIO().WantCaptureMouse && GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
        mouse_over == selected_planet && !scene::IsGameHalted() && !GetApp().MouseDragged()) {
        to_see = true;

        if (GetUniverse().valid(selected_planet)) {
            SPDLOG_INFO("Switched entity: {}", GetUniverse().get<components::Identifier>(selected_planet).identifier);
        } else {
            SPDLOG_INFO("Switched entity is not valid");
        }
    }
    if (!GetUniverse().valid(selected_planet) || !GetUniverse().all_of<bodies::Body>(selected_planet)) {
        to_see = false;
    }
}
void SysPlanetMarketInformation::LocalMarketInformation() {
    if (!ImGui::BeginTabItem("Market Information")) {
        return;
    }
    if (GetUniverse().any_of<components::Wallet>(selected_planet)) {
        auto& wallet = GetUniverse().get<components::Wallet>(selected_planet);
        ImGui::TextFmt("GDP Contribution: {}", util::NumberToHumanString(wallet.GetGDPChange()));
        ImGui::TextFmt("Balance: {}", util::NumberToHumanString(wallet.GetBalance()));
        ImGui::TextFmt("Balance change: {}", util::NumberToHumanString(wallet.GetChange()));
    }
    MarketInformationTable(GetUniverse(), selected_planet);
    ImGui::EndTabItem();
}

void SysPlanetMarketInformation::InterplanetaryTradeInformation() {
    if (!ImGui::BeginTabItem("Trade Information")) {
        return;
    }
    if (!GetUniverse().any_of<components::PlanetaryMarket>(selected_planet)) {
        ImGui::Text("No Planetary Market!");
        ImGui::EndTabItem();
        return;
    }
    auto& interplanetary_market = GetUniverse().get<components::PlanetaryMarket>(selected_planet);
    ImGui::BeginChild("planetary_market_left", ImVec2(300, 700));
    ImGui::InputText("##planetary_market_search_text", search_text.data(), search_text.size());
    std::string search_string(search_text.data());
    std::transform(search_string.begin(), search_string.end(), search_string.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    ImGui::BeginChild("planetary_trade_viewer_scroll");
    for (auto& [good, order] : interplanetary_market.demands) {
        bool is_selected = selected_good == good;
        std::string name = common::util::GetName(GetUniverse(), good);
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!search_string.empty()) {
            // Then we can check if the text contains it
            if (name_lower.find(search_string) == std::string::npos) {
                continue;
            }
        }
        // Now check if the string is in stuff
        if (ImGui::SelectableFmt("{}", &is_selected, name)) {
            selected_good = good;
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("planetary_trade_viewer_right", ImVec2(400, 700));
    InterplanetaryTradeRightPanel();
    ImGui::EndChild();
    ImGui::EndTabItem();
}

void SysPlanetMarketInformation::InterplanetaryTradeRightPanel() {
    // Get the list of selected goods
    auto& interplanetary_market = GetUniverse().get<components::PlanetaryMarket>(selected_planet);
    for (auto& demand : interplanetary_market.demands[selected_good]) {
        ImGui::TextFmt("Target: {}", common::util::GetName(GetUniverse(), demand.target));
        ImGui::TextFmt("Amount: {}", demand.amount);
        ImGui::TextFmt("Price: {}", demand.price);
        ImGui::Separator();
    }
}

void SysPlanetMarketInformation::InterplanetaryTradeAmounts() {
    if (!ImGui::BeginTabItem("Intraplanetary Trade Details")) {
        return;
    }
    if (!GetUniverse().any_of<components::PlanetaryMarket>(selected_planet)) {
        ImGui::Text("No Planetary Market!");
        ImGui::EndTabItem();
        return;
    }
    // Show a resource ledger
    // Get resource stockpile
    if (!ImGui::BeginTable("supply table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::EndTabItem();
        return;
    }
    ImGui::TableSetupColumn("Good");
    ImGui::TableSetupColumn("Additional Supply");

    ImGui::TableHeadersRow();
    auto goodsview = GetUniverse().view<components::Price>();
    auto& interplanetary_market = GetUniverse().get<components::PlanetaryMarket>(selected_planet);
    for (entt::entity good_entity : goodsview) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (GetUniverse().any_of<components::CapitalGood>(good_entity)) {
            ImGui::TextFmtColored(ImColor(1.f, 1.f, 0.f), "{}", GetName(GetUniverse(), good_entity));
        } else {
            ImGui::TextFmt("{}", GetName(GetUniverse(), good_entity));
        }
        if (ImGui::IsItemHovered()) {
            gui::EntityTooltip(GetUniverse(), good_entity);
        }
        ImGui::TableSetColumnIndex(1);
        // Mark the cell as red if the thing is not valid
        ImGui::TextFmt("{}", interplanetary_market.supply_difference[good_entity]);
    }
    ImGui::EndTable();
    ImGui::EndTabItem();
}

void SysPlanetMarketInformation::PerGoodDetails() {
    if (!GetUniverse().any_of<components::PlanetaryMarket>(selected_planet)) {
        ImGui::Text("No Planetary Market!");
        ImGui::EndTabItem();
        return;
    }
    auto& interplanetary_market = GetUniverse().get<components::PlanetaryMarket>(selected_planet);
    ImGui::BeginChild("per_good_left", ImVec2(300, 700));
    ImGui::InputText("##per_good_search_text", per_good_details_search_text.data(), per_good_details_search_text.size());
    std::string search_string(per_good_details_search_text.data());
    std::transform(search_string.begin(), per_good_details_search_text.end(), per_good_details_search_text.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    ImGui::BeginChild("per_good_viewer_scroll");
    for (auto& [good, order] : interplanetary_market.demands) {
        bool is_selected = per_good_details_selected == good;
        std::string name = common::util::GetName(GetUniverse(), good);
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!search_string.empty()) {
            // Then we can check if the text contains it
            if (name_lower.find(search_string) == std::string::npos) {
                continue;
            }
        }
        // Now check if the string is in stuff
        if (ImGui::SelectableFmt("{}", &is_selected, name)) {
            per_good_details_selected = good;
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("per_good_viewer_right", ImVec2(400, 700));
    PerGoodDetailsRightPanel();
    ImGui::EndChild();
    ImGui::EndTabItem();
}

void SysPlanetMarketInformation::PerGoodDetailsRightPanel() {
    // Now add the tables for each city and stuff
    if (!GetUniverse().any_of<components::Habitation>(selected_planet)) {
        ImGui::TextFmt("No cities attached!");
        return;
    }
    if (!ImGui::BeginTable("habitation table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        return;
    }
    ImGui::TableSetupColumn("City");
    ImGui::TableSetupColumn("Additional Supply");
    auto& cities = GetUniverse().get<components::Habitation>(selected_planet);
    for (entt::entity city : cities.settlements) {
        auto& market = GetUniverse().get<components::Market>(city);
    }
    ImGui::EndTable();
}
}  // namespace cqsp::client::systems
