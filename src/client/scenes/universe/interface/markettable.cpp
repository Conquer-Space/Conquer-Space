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
#include "client/scenes/universe/interface/markettable.h"

#include <imgui.h>

#include "client/scenes/universe/interface/systooltips.h"
#include "core/components/bodies.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/gui.h"

namespace cqsp::client::systems {
namespace bodies = cqsp::core::components::bodies;
namespace components = cqsp::core::components;

using cqsp::core::util::GetName;

void MarketInformationTable(core::Universe& universe, const entt::entity& market_entity) {
    if (!universe.any_of<components::Market>(market_entity)) {
        ImGui::TextFmt("Market is not a market");
        return;
    }
    // auto& center = GetUniverse().get<cqspc::MarketCenter>(marketentity);
    components::Market& market = universe.get<components::Market>(market_entity);
    ImGui::TextFmt("Has {} entities attached to it", market.participants.size());

    // Get resource stockpile
    if (!ImGui::BeginTable("marketinfotable", 11, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        return;
    }
    ImGui::TableSetupColumn("Good");
    ImGui::TableSetupColumn("Price");
    ImGui::TableSetupColumn("Supply");
    ImGui::TableSetupColumn("Demand");
    ImGui::TableSetupColumn("S/D ratio");
    ImGui::TableSetupColumn("Production");
    ImGui::TableSetupColumn("Consumption");
    ImGui::TableSetupColumn("Trade");
    ImGui::TableSetupColumn("Market Access");
    ImGui::TableSetupColumn("Chronic Shortages");
    ImGui::TableSetupColumn("Resource Fulfilled");

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
        if (ImGui::IsItemHovered()) {
            gui::EntityTooltip(universe, good_entity);
        }
        ImGui::TableSetColumnIndex(1);
        // Mark the cell as red if the thing is not valid
        ImGui::TextFmt("{}", market.price[good_entity]);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextFmt("{}", cqsp::util::NumberToHumanString(market.supply()[good_entity]));
        ImGui::TableSetColumnIndex(3);
        ImGui::TextFmt("{}", cqsp::util::NumberToHumanString(market.demand()[good_entity]));
        ImGui::TableSetColumnIndex(4);
        double sd_ratio = market.sd_ratio[good_entity];
        if (sd_ratio == std::numeric_limits<double>::infinity()) {
            ImGui::TextFmt("inf");
        } else {
            ImGui::TextFmt("{}", sd_ratio);
        }
        ImGui::TableSetColumnIndex(5);
        ImGui::TextFmt("{}", cqsp::util::NumberToHumanString(market.production[good_entity]));
        ImGui::TableSetColumnIndex(6);
        ImGui::TextFmt("{}", cqsp::util::NumberToHumanString(market.consumption[good_entity]));
        ImGui::TableSetColumnIndex(7);
        ImGui::TextFmt("{}", cqsp::util::NumberToHumanString(market.trade[good_entity]));
        ImGui::TableSetColumnIndex(8);
        ImGui::TextFmt("{}", (market.market_access[good_entity]));
        ImGui::TableSetColumnIndex(9);
        ImGui::TextFmt("{}", (market.chronic_shortages[good_entity]));
        ImGui::TableSetColumnIndex(10);
        ImGui::TextFmt("{}", (market.resource_fulfilled[good_entity]));
    }
    ImGui::EndTable();
}
}  // namespace cqsp::client::systems
