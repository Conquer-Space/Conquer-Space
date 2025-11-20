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
using core::components::ResourceLedger;
using util::NumberToHumanString;

bool DrawLedgerTable(const std::string &name, const Universe &universe, const ResourceLedger &ledger) {
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

}  // namespace cqsp::client::systems