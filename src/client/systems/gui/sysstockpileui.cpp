#include "client/systems/gui/sysstockpileui.h"

#include <fmt/format.h>

#include "common/components/name.h"
#include "common/util/utilnumberdisplay.h"
#include "engine/gui.h"

using conquerspace::common::components::Universe;
using conquerspace::common::components::Identifier;
using conquerspace::util::LongToHumanString;
using conquerspace::common::components::ResourceLedger;
void conquerspace::client::systems::DrawLedgerTable(const std::string &name, Universe &universe, ResourceLedger& ledger) {
    if (ledger.empty()) {
        ImGui::Text("Empty ledger");
        return;
    }
    if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableHeadersRow();
        for (auto iterator = ledger.begin();
                                    iterator != ledger.end(); iterator++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", universe.get<Identifier>(iterator->first).identifier);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", LongToHumanString(static_cast<int64_t>(iterator->second)));
        }
        ImGui::EndTable();
    }
}
