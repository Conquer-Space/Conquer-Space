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
#include "client/scenes/universe/interface/modifiertooltip.h"

#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/gui.h"

namespace cqsp::client::systems::gui {
using util::NumberToHumanString;
void ModifierTooltip(const core::Universe& universe, const core::components::AdditiveModifier& modifier) {
    if (!ImGui::IsItemHovered()) {
        return;
    }

    ImGui::BeginTooltip();
    ImGui::TextFmt("Value: {}", NumberToHumanString(modifier));
    ImGui::TextFmt("Modifier: {}", modifier.sum);
    // Then loop through...
    for (auto& [entity, amount] : modifier.modifiers) {
        if (amount > 0) {
            ImGui::TextFmt("{}: +{}", core::util::GetName(universe, entity), amount);
        } else {
            ImGui::TextFmt("{}: {}", core::util::GetName(universe, entity), amount);
        }
    }
    ImGui::EndTooltip();
}

void ModifierTooltip(const core::Universe& universe, const core::components::MultiplicativeModifier& modifier) {
    if (!ImGui::IsItemHovered()) {
        return;
    }

    ImGui::BeginTooltip();
    ImGui::TextFmt("Value: {}", NumberToHumanString(modifier));
    ImGui::TextFmt("Modifier: {}%", (modifier.sum - 1) * 100);
    // Then loop through...
    for (auto& [entity, amount] : modifier.modifiers) {
        if (amount > 0) {
            ImGui::TextFmt("{}: +{}", core::util::GetName(universe, entity), amount * 100);
        } else {
            ImGui::TextFmt("{}: {}", core::util::GetName(universe, entity), amount * 100);
        }
    }
    ImGui::EndTooltip();
}
}  // namespace cqsp::client::systems::gui
