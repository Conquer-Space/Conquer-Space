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
#pragma once

#include <imgui.h>

#include <string>
#include <vector>

#include <entt/entt.hpp>

#include "core/components/resource.h"
#include "core/universe.h"

namespace cqsp::client::systems {

double GetLedgerCost(core::Universe& universe, const core::components::ResourceVector& ledger);

bool GoodCombo(const char* label, int& selected_idx, const std::vector<std::string>& list);

void IdentifierTooltipOnItem(core::Universe& universe, entt::entity entity, const ImVec4& color);

void ResourceVectorSection(core::Universe& universe, core::components::ResourceVector& ledger,
                           const std::vector<std::string>& good_list, int& new_good_idx, const char* table_id,
                           const char* amount_col, const char* combo_id, const char* add_label,
                           const ImVec4& id_copy_color);

}  // namespace cqsp::client::systems
