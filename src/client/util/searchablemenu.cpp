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
#include "client/util/searchablemenu.h"

#include "core/util/nameutil.h"
#include "engine/cqspgui.h"

namespace cqsp::client::systems {
void SearchableMenu::LeftMenu(const std::string& name, core::Universe& universe,
                              const std::vector<entt::entity>& list) {
    ImGui::BeginChild(fmt::format("{}_viewer_left", name).c_str(), ImVec2(300, 700));
    ImGui::InputText(fmt::format("##{}_viewer_search_text", name).c_str(), search_text.data(), search_text.size());
    std::string search_string(search_text.data());
    std::transform(search_string.begin(), search_string.end(), search_string.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    ImGui::BeginChild(fmt::format("{}_viewer_scroll", name).c_str());
    int idx = 0;
    for (entt::entity entity : list) {
        bool is_selected = entity == selected_item;
        std::string name = core::util::GetName(universe, entity);
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!search_string.empty()) {
            // Then we can check if the text contains it
            if (name_lower.find(search_string) == std::string::npos) {
                continue;
            }
        }
        if (ImGui::SelectableFmt("{}###{}", &is_selected, name, idx)) {
            selected_item = entity;
        }
        idx++;
    }
    ImGui::EndChild();
    ImGui::EndChild();
}

void SearchableMenu::Display(const std::string& name, core::Universe& universe, const std::vector<entt::entity>& list) {
    LeftMenu(name, universe, list);
}
}  // namespace cqsp::client::systems
