/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "client/systems/editor/recipeviewer.h"

#include "client/systems/gui/systooltips.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/resource.h"

void cqsp::client::systems::SysRecipeViewer::Init() {}

void cqsp::client::systems::SysRecipeViewer::DoUI(int delta_time) {
    ImGui::Begin("Recipe Viewer");
    // List out all the stuff
    auto recipes = GetUniverse().view<common::components::Recipe>();
    ImGui::TextFmt("Recipes: {}", recipes.size());
    ImGui::BeginChild("recipe_viewer_left", ImVec2(300, -1));
    for (entt::entity recipe : recipes) {
        bool is_selected = recipe == selected_recipe;
        if (ImGui::SelectableFmt("{}", &is_selected, gui::GetName(GetUniverse(), recipe))) {
            selected_recipe = recipe;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("recipe_viewer_right", ImVec2(300, -1));
    RecipeViewerRight();
    ImGui::EndChild();
    ImGui::End();
}

void cqsp::client::systems::SysRecipeViewer::DoUpdate(int delta_time) {}

namespace {
void ResourceMapTable(cqsp::common::Universe& universe, cqsp::common::components::ResourceLedger& ledger,
                      const char* name) {
    if (!ImGui::BeginTable(name, 2)) {
        return;
    }
    ImGui::TableSetupColumn("Good");
    ImGui::TableSetupColumn("Amount");
    ImGui::TableHeadersRow();
    for (auto& in : ledger) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextFmt("{}", cqsp::client::systems::gui::GetName(universe, in.first));
        ImGui::TableSetColumnIndex(1);
        ImGui::TextFmt("{}", in.second);
    }
    ImGui::EndTable();
}
}  // namespace

void cqsp::client::systems::SysRecipeViewer::RecipeViewerRight() {
    namespace cc = common::components;
    if (selected_recipe == entt::null) {
        ImGui::Text("Good is invalid!");
        return;
    }
    ImGui::TextFmt("Name: {}", gui::GetName(GetUniverse(), selected_recipe));
    ImGui::TextFmt("Identifier: {}", GetUniverse().get<cc::Identifier>(selected_recipe).identifier);
    // Get inputs and outputs
    auto& recipe_comp = GetUniverse().get<cc::Recipe>(selected_recipe);
    ImGui::TextFmt("Workers per unit of recipe: {}", recipe_comp.workers);
    ImGui::Text("Input");
    ResourceMapTable(GetUniverse(), recipe_comp.input, "input_table");
    ImGui::Text("Capital Cost");
    ResourceMapTable(GetUniverse(), recipe_comp.capitalcost, "capital_table");
    ImGui::Text("Output");
    ImGui::TextFmt("{}, {}", gui::GetName(GetUniverse(), recipe_comp.output.entity), recipe_comp.output.amount);
}
