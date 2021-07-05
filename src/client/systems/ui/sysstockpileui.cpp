#include "sysstockpileui.h"

#include <fmt/format.h>

#include "common/components/name.h"
#include "common/util/utilnumberdisplay.h"
#include "engine/gui.h"

using conquerspace::common::components::Universe;
using conquerspace::common::components::Identifier;
void conquerspace::client::systems::DrawLedgerTable(Universe &universe, conquerspace::common::components::ResourceLedger& ledger) {
    if (ImGui::BeginTable("", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Good");
            ImGui::TableSetupColumn("Amount");
            ImGui::TableHeadersRow();
            for (auto iterator = ledger.begin();
                                        iterator != ledger.end(); iterator++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(fmt::format("{}", universe
                            .get<Identifier>(iterator->first).identifier).c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(fmt::format("{}", conquerspace::util::LongToHumanString(static_cast<int64_t>(iterator->second))).c_str());
            }
            ImGui::EndTable();
        }
}
