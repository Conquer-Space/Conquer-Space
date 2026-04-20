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
#include "client/scenes/universe/interface/productionsummary.h"

#include "client/scenes/universe/interface/systooltips.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"

namespace cqsp::client::systems {
namespace components = cqsp::core::components;
using util::NumberToHumanString;

void ProductionSummary::Init() {}

void ProductionSummary::DoUI(int delta_time) {
    ImGui::Begin("Production Summary");
    std::vector<ImVec4> colors = {ImVec4(1, 0, 0, 1), ImVec4(0, 1, 0, 1),   ImVec4(0, 0, 1, 1),
                                  ImVec4(1, 1, 0, 1), ImVec4(1, 0, 1, 1),   ImVec4(0, 1, 1, 1),
                                  ImVec4(1, 1, 1, 1), ImVec4(1, 0.5, 0, 1), ImVec4(1, 1, 0.5, 1)};
    if (ImGui::BeginTable("industry_list_table", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Production Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Utilization");
        ImGui::TableSetupColumn("Utilization Delta");
        ImGui::TableSetupColumn("Cash Reserves");
        ImGui::TableSetupColumn("Hired Workers");
        ImGui::TableSetupColumn("Wage");
        ImGui::TableSetupColumn("Revenue");
        ImGui::TableSetupColumn("Profit");
        ImGui::TableHeadersRow();
        for (auto&& [industry, industry_component] : GetUniverse().view<core::components::ProductionUnit>().each()) {
            ImGui::TableNextRow();
            // Then now we should show a row or something
            ImGui::TableSetColumnIndex(0);
            // Fix this so that this doesn't die lol
            ImGui::TextFmtColored(colors[static_cast<uint32_t>(industry_component.state)], "{}",
                                  core::util::GetName(GetUniverse(), industry));
            if (ImGui::IsItemHovered()) {
                systems::gui::EntityTooltip(GetUniverse(), industry);
            }

            ImGui::TableSetColumnIndex(1);
            if (!industry_component.shortage) {
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.size)));
            } else {
                ImGui::TextFmtColored(ImVec4(0.75, 0, 0, 1), "{}",
                                      NumberToHumanString(static_cast<int64_t>(industry_component.size)));
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextFmt("Resource Shortage!");
                    ImGui::EndTooltip();
                }
            }
            ImGui::TableSetColumnIndex(2);
            if (GetUniverse().all_of<components::Construction>(industry)) {
                auto& construction = GetUniverse().get<components::Construction>(industry);
                ImGui::ProgressBar(static_cast<float>(construction.progress) /
                                   static_cast<float>(construction.maximum));
            } else {
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.utilization)));
            }
            ImGui::TableSetColumnIndex(3);
            double diff = industry_component.diff - 1;
            diff *= 100;
            const char* format_string = "{:.2f}%";
            if (diff > 0) {
                ImGui::TextFmtColored(ImVec4(0, 0.75, 0, 1), fmt::runtime(format_string), diff);
            } else if (diff < 0) {
                ImGui::TextFmtColored(ImVec4(0.75, 0, 0, 1), fmt::runtime(format_string), diff);
            } else {
                ImGui::TextFmt(fmt::runtime(format_string), diff);
            }

            if (GetUniverse().all_of<components::Wallet>(industry)) {
                auto& wallet = GetUniverse().get<components::Wallet>(industry);
                ImGui::TableSetColumnIndex(4);
                ImGui::TextFmt("{}", NumberToHumanString(wallet.GetBalance()));
            }
            if (GetUniverse().all_of<components::Employer>(industry)) {
                ImGui::TableSetColumnIndex(5);
                auto& employer = GetUniverse().get<components::Employer>(industry);
                ImGui::TextFmt("{}/{}", NumberToHumanString(static_cast<int64_t>(employer.population_change)),
                               NumberToHumanString(static_cast<int64_t>(employer.population_fufilled)));
            }

            ImGui::TableSetColumnIndex(6);
            ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.wages)));

            ImGui::TableSetColumnIndex(7);
            ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.revenue)));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextFmt("Items sold: {}", industry_component.amount_sold);
                ImGui::EndTooltip();
            }
            ImGui::TableSetColumnIndex(8);
            ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.profit)));
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void ProductionSummary::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
