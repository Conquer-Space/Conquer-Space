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
#include "client/scenes/universe/interface/taxwindow.h"

#include "core/components/name.h"

namespace cqsp::client::systems {
void TaxWindow::Init() {
    // Then we should init our goods
    // GetUniverse().GoodIterator()
    tax_rates.resize(GetUniverse().GoodCount());
}

void TaxWindow::DoUI(int delta_time) {
    // Now we can set the taxes or something
    ImGui::Begin("Taxes");
    if (ImGui::BeginTable("tax_table", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Tax (Not a rate it's just absolute value)");
        ImGui::TableHeadersRow();
        int idx = 0;
        for (auto good : GetUniverse().GoodIterator()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            std::string name = GetUniverse().get<core::components::Name>(good).name;
            ImGui::TextFmt("{}", name);
            ImGui::TableSetColumnIndex(1);
            if (ImGui::SliderFloat(("###tax_slider" + name).c_str(), &tax_rates[idx], 0, 5.)) {
                // Update all the market taxes..
                // Then
                for (auto&& [entity, zone, market] :
                     GetUniverse().view<core::components::IndustrialZone, core::components::Market>().each()) {
                    // Now just add taxes
                    market.taxation[good] = tax_rates[idx];
                }
            }
            idx++;
        }
        ImGui::EndTable();
    }
    ImGui::End();
}
void TaxWindow::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
