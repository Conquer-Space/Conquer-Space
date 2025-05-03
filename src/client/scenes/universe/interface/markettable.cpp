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

#include "common/components/bodies.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"
#include "engine/gui.h"

namespace cqsp::client::systems {
namespace cqspb = cqsp::common::components::bodies;
namespace cqspc = cqsp::common::components;

using cqsp::common::util::GetName;

void MarketInformationTable(common::Universe& universe, const entt::entity& market_entity) {
    if (!universe.any_of<cqspc::Market>(market_entity)) {
        ImGui::TextFmt("Market is not a market");
        return;
    }
    // auto& center = GetUniverse().get<cqspc::MarketCenter>(marketentity);
    cqspc::Market& market = universe.get<cqspc::Market>(market_entity);
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
    auto goodsview = universe.view<cqspc::Price>();

    for (entt::entity good_entity : goodsview) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (universe.any_of<cqspc::CapitalGood>(good_entity)) {
            ImGui::TextFmtColored(ImColor(1.f, 1.f, 0.f), "{}", GetName(universe, good_entity));
        } else {
            ImGui::TextFmt("{}", GetName(universe, good_entity));
        }
        ImGui::TableSetColumnIndex(1);
        // Mark the cell as red if the thing is not valid
        ImGui::TextFmt("{}", market.price[good_entity]);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextFmt("{}", cqsp::util::LongToHumanString(market.previous_supply()[good_entity]));
        ImGui::TableSetColumnIndex(3);
        ImGui::TextFmt("{}", cqsp::util::LongToHumanString(market.previous_demand()[good_entity]));
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
}  // namespace cqsp::client::systems
