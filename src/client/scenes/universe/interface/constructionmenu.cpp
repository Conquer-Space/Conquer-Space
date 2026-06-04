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
#include "client/scenes/universe/interface/constructionmenu.h"

#include "client/components/clientctx.h"
#include "core/actions/factoryconstructaction.h"
#include "core/components/area.h"
#include "core/components/name.h"
#include "core/components/resource.h"
#include "core/components/surface.h"

namespace cqsp::client::systems {
namespace components = core::components;

bool ConstructionMenu::RecipeCombo(const char* label, int& selected_idx,
                                   std::vector<std::pair<std::string, entt::entity>>& list) {
    if (list.empty()) return false;
    if (selected_idx >= static_cast<int>(list.size())) {
        selected_idx = 0;
    }
    bool changed = false;
    if (ImGui::BeginCombo(label, list[selected_idx].first.c_str())) {
        if (ImGui::IsWindowAppearing()) {
            search_text.fill(0);
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##combo_search", search_text.data(), search_text.size());
        std::string search_lower(search_text.data());
        std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        for (int i = 0; i < static_cast<int>(list.size()); i++) {
            if (!search_lower.empty()) {
                std::string item_lower = list[i].first;
                std::transform(item_lower.begin(), item_lower.end(), item_lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                if (item_lower.find(search_lower) == std::string::npos) continue;
            }
            bool is_selected = i == selected_idx;
            if (ImGui::Selectable(list[i].first.c_str(), is_selected)) {
                selected_idx = i;
                changed = true;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

void ConstructionMenu::PerformConstruction(int count) {
    entt::entity province = GetUniverse().ctx().at<client::ctx::HoveredProvince>().hovered_province;
    if (!GetUniverse().valid(province) ||
        !GetUniverse().all_of<components::Province, components::IndustrialZone>(province)) {
        return;
    }
    if (GetUniverse().get<components::Province>(province).country != GetUniverse().GetPlayer()) {
        return;
    }
    // then we can build
    // Check if the player owns the province
    auto& industrial_zone = GetUniverse().get<components::IndustrialZone>(province);
    if (industrial_zone.industries.empty()) {
        // Then add stuff
        // Create industry
        // Add to city
        auto factory = core::actions::CreateFactory(GetUniverse()(province),
                                                    GetUniverse()(recipe_list[selected_index].second), count);
    } else {
        // check our selected thing
        entt::entity back = industrial_zone.industries.back();
        // then we add stuff to the size
        GetUniverse().get<components::ProductionUnit>(back).size += count;
    }
}

void ConstructionMenu::Init() {
    for (auto&& [entity, name, recipe] : GetUniverse().view<components::Name, components::Recipe>().each()) {
        recipe_list.emplace_back(name, entity);
    }
}

void ConstructionMenu::DoUI(int delta_time) {
    ImGui::Begin("Construction menu");
    // Now we have a drop down menu for everything and stuff
    // Now compute the other stuff
    ImGui::Text(
        "Click on a province to add a building, shift click for 5, control click for 10 and both for 25 (or something "
        "we'll see)");
    RecipeCombo("###selected_recipe_construction", selected_index, recipe_list);
    ImGui::SameLine();
    if (ImGui::Button((constructing) ? "Constructing..." : "Construct!")) {
        constructing = !constructing;
    }
    ImGui::End();
}

void ConstructionMenu::DoUpdate(int delta_time) {
    if (!ImGui::GetIO().WantCaptureKeyboard && constructing) {
        int production_count = 1;
        if (GetApp().ButtonIsHeld(engine::KeyInput::KEY_LEFT_SHIFT)) {
            // Construct something
            // Get the currently hovering province then we can add more stuff
            production_count = 5;
        } else if (GetApp().ButtonIsHeld(engine::KeyInput::KEY_LEFT_CONTROL)) {
            production_count = 10;
        }
        if (GetApp().MouseButtonIsPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            PerformConstruction(production_count);
        }

        if (constructing && GetApp().ButtonIsReleased(engine::KeyInput::KEY_ESCAPE)) {
            // then let's
            constructing = false;
        }
    }
}
}  // namespace cqsp::client::systems
