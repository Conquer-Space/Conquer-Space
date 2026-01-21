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
#include "sysstockpileui.h"

#include "core/components/name.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/gui.h"
#include "fmt/format.h"
#include "systooltips.h"

namespace cqsp::client::systems {

using core::Universe;
using core::components::Identifier;
using core::components::ResourceMap;
using util::NumberToHumanString;

bool DrawLedgerTable(const std::string &name, const Universe &universe, const ResourceMap &ledger) {
    if (ledger.empty()) {
        ImGui::Text("Empty ledger");
        return false;
    }
    if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableHeadersRow();
        for (auto iterator = ledger.begin(); iterator != ledger.end(); iterator++) {
            if (iterator->second == 0) {
                continue;
            }
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(universe, iterator->first));
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(iterator->second)));
        }
        ImGui::EndTable();
    }
    return true;
}

bool DrawLedgerTable(const std::string &name, const Universe &universe, const ResourceMap &ledger,
                     const core::components::Market &market) {
    if (ledger.empty()) {
        ImGui::Text("Empty ledger");
        return false;
    }
    if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableHeadersRow();
        for (auto iterator = ledger.begin(); iterator != ledger.end(); iterator++) {
            if (iterator->second == 0) {
                continue;
            }
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(universe, iterator->first));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextFmt("Good Price: {}", market.price[iterator->first]);
                ImGui::TextFmt("Combined Price: {}",
                               NumberToHumanString(market.price[iterator->first] * iterator->second));
                ImGui::EndTooltip();
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(iterator->second)));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextFmt("Good Price: {}", market.price[iterator->first]);
                ImGui::TextFmt("Combined Price: {}",
                               NumberToHumanString(market.price[iterator->first] * iterator->second));
                ImGui::EndTooltip();
            }
        }
        ImGui::EndTable();
    }
    return true;
}

bool DrawLedgerPiePlot(const std::string &name, const core::Universe &universe,
                       const core::components::ResourceMap &ledger, const core::components::Market &market,
                       bool price) {
    if (ImPlot::BeginPlot("##Pie1", ImVec2(ImGui::GetTextLineHeight() * 16, ImGui::GetTextLineHeight() * 16),
                          ImPlotFlags_Equal | ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(0, 1, 0, 1);
        // If it's by price or by something else
        std::vector<const char *> labels;
        std::vector<std::string> label_strings;
        std::vector<int> values;
        for (auto iterator = ledger.begin(); iterator != ledger.end(); iterator++) {
            label_strings.push_back(core::util::GetName(universe, iterator->first));
            if (price) {
                values.push_back(iterator->second * market.price[iterator->first]);
            } else {
                values.push_back(iterator->second);
            }
        }

        for (const std::string &label : label_strings) {
            labels.push_back(label.c_str());
        }
        ImPlot::PlotPieChart(labels.data(), values.data(), label_strings.size(), 0.5, 0.5, 0.4, "%.2f", 90,
                             ImPlotPieChartFlags_Normalize);
        ImPlot::EndPlot();
    }
    return false;
}

}  // namespace cqsp::client::systems
