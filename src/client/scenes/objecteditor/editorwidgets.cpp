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
#include "client/scenes/objecteditor/editorwidgets.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "core/components/market.h"
#include "core/components/name.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/cqspgui.h"

namespace cqsp::client::systems {

double GetLedgerCost(core::Universe& universe, const core::components::ResourceVector& ledger) {
    double cost = 0;
    for (auto& [entity, amount] : ledger) {
        cost += universe.get<core::components::Price>(entity) * amount;
    }
    return cost;
}

bool GoodCombo(const char* label, int& selected_idx, const std::vector<std::string>& list) {
    if (list.empty()) return false;
    if (selected_idx >= static_cast<int>(list.size())) selected_idx = 0;
    bool changed = false;
    if (ImGui::BeginCombo(label, list[selected_idx].c_str())) {
        static char search_buf[128] = {};
        if (ImGui::IsWindowAppearing()) {
            memset(search_buf, 0, sizeof(search_buf));
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##combo_search", search_buf, sizeof(search_buf));
        std::string search_lower(search_buf);
        std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        for (int i = 0; i < static_cast<int>(list.size()); i++) {
            if (!search_lower.empty()) {
                std::string item_lower = list[i];
                std::transform(item_lower.begin(), item_lower.end(), item_lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                if (item_lower.find(search_lower) == std::string::npos) continue;
            }
            bool is_selected = (i == selected_idx);
            if (ImGui::Selectable(list[i].c_str(), is_selected)) {
                selected_idx = i;
                changed = true;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

void IdentifierTooltipOnItem(core::Universe& universe, entt::entity entity, const ImVec4& color) {
    if (ImGui::IsItemClicked()) {
        ImGui::SetClipboardText(universe.get<core::components::Identifier>(entity).identifier.c_str());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(color, "Click to copy identifier");
        ImGui::EndTooltip();
    }
}

void ResourceVectorSection(core::Universe& universe, core::components::ResourceVector& ledger,
                           const std::vector<std::string>& good_list, int& new_good_idx, const char* table_id,
                           const char* amount_col, const char* combo_id, const char* add_label,
                           const ImVec4& id_copy_color) {
    ImGui::TextFmt("Default Cost: {}", util::NumberToHumanString(GetLedgerCost(universe, ledger)));
    int erase_idx = -1;
    if (ImGui::BeginTable(table_id, 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn(amount_col);
        ImGui::TableSetupColumn("##del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        int i = 0;
        for (auto& [entity, amount] : ledger) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(universe, entity));
            IdentifierTooltipOnItem(universe, universe.GetGood(entity), id_copy_color);
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            ImGui::PushID(i);
            ImGui::InputDouble("##amount", &amount);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("-")) erase_idx = i;
            ImGui::PopID();
            i++;
        }
        ImGui::EndTable();
    }
    if (erase_idx >= 0) ledger.erase(ledger.begin() + erase_idx);
    ImGui::SetNextItemWidth(180);
    GoodCombo(combo_id, new_good_idx, good_list);
    ImGui::SameLine();
    if (ImGui::Button(add_label) && !good_list.empty()) {
        core::components::GoodEntity ge = universe.good_map[universe.goods[good_list[new_good_idx]]];
        ledger.push_back({ge, 1.0});
        ledger.Finalize();
    }
}

}  // namespace cqsp::client::systems
