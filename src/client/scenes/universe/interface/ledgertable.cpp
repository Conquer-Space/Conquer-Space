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
#include "client/scenes/universe/interface/ledgertable.h"

#include "client/scenes/universe/interface/systooltips.h"
#include "core/components/name.h"
#include "core/util/nameutil.h"
#include "engine/gui.h"

namespace cqsp::client::systems {
void ResourceMapTable(core::Universe& universe, core::components::ResourceMap& ledger, const char* name) {
    const ImVec4 id_copy_color = ImVec4(0.921568627f, 0.392156863f, 0.203921569f, 1.f);
    if (!ImGui::BeginTable(name, 2)) {
        return;
    }
    ImGui::TableSetupColumn("Good");
    ImGui::TableSetupColumn("Amount");
    ImGui::TableHeadersRow();
    for (auto& in : ledger) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextFmt("{}", cqsp::core::util::GetName(universe, in.first));
        if (ImGui::IsItemClicked()) {
            // Copy
            ImGui::SetClipboardText(universe.get<core::components::Identifier>(in.first).identifier.c_str());
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextColored(id_copy_color, "Click to copy identifier");
            systems::gui::EntityTooltipContent(universe, in.first);
            ImGui::EndTooltip();
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextFmt("{}", in.second);
    }
    ImGui::EndTable();
}
}  // namespace cqsp::client::systems
