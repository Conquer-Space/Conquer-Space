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
#include "recipeviewer.h"

#include "client/scenes/universe/interface/systooltips.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/nameutil.h"

namespace cqsp::client::systems {

namespace components = common::components;

void SysRecipeViewer::Init() {}

void SysRecipeViewer::DoUI(int delta_time) {
    ImGui::Begin("Recipe Viewer");
    // List out all the stuff
    auto recipes = GetUniverse().view<components::Recipe>();
    ImGui::TextFmt("Recipes: {}", recipes.size());
    ImGui::BeginChild("recipe_viewer_left", ImVec2(300, 700));
    for (entt::entity recipe : recipes) {
        bool is_selected = recipe == selected_recipe;
        if (ImGui::SelectableFmt("{}", &is_selected, common::util::GetName(GetUniverse(), recipe))) {
            selected_recipe = recipe;
            ResetSelection();
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("recipe_viewer_right", ImVec2(400, 700));
    RecipeViewerRight();
    ImGui::EndChild();
    ImGui::End();
}

void SysRecipeViewer::DoUpdate(int delta_time) {}

namespace {
void ResourceMapTable(common::Universe& universe, components::ResourceLedger& ledger, 
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
        ImGui::TextFmt("{}", cqsp::common::util::GetName(universe, in.first));
        ImGui::TableSetColumnIndex(1);
        ImGui::TextFmt("{}", in.second);
    }
    ImGui::EndTable();
}

double GetLedgerCost(common::Universe& universe, components::ResourceLedger& ledger) {
    double input_cost = 0;
    for (auto& [entity, amount] : ledger) {
        input_cost = universe.get<components::Price>(entity) * amount;
    }
    return input_cost;
}
}  // namespace

void SysRecipeViewer::RecipeViewerRight() {
    if (selected_recipe == entt::null) {
        ImGui::Text("Good is invalid!");
        return;
    }
    ImGui::TextFmt("Name: {}", common::util::GetName(GetUniverse(), selected_recipe));
    ImGui::TextFmt("Identifier: {}", GetUniverse().get<components::Identifier>(selected_recipe).identifier);
    // Get inputs and outputs
    auto& recipe_comp = GetUniverse().get<components::Recipe>(selected_recipe);
    ImGui::TextFmt("Workers per unit of recipe: {}", recipe_comp.workers);
    ImGui::Text("Input");
    ImGui::TextFmt("Input Default Cost: {}", GetLedgerCost(GetUniverse(), recipe_comp.input));
    ResourceMapTable(GetUniverse(), recipe_comp.input, "input_table");
    ImGui::Separator();
    ImGui::Text("Capital Cost");
    ImGui::TextFmt("Capital Default Cost: {}", GetLedgerCost(GetUniverse(), recipe_comp.capitalcost));
    ResourceMapTable(GetUniverse(), recipe_comp.capitalcost, "capital_table");
    ImGui::Separator();
    ImGui::Text("Output");
    ImGui::TextFmt("Output Cost: {}", GetUniverse().get<components::Price>(recipe_comp.output.entity) * recipe_comp.output.amount);
    ImGui::TextFmt("{}, {}", common::util::GetName(GetUniverse(), recipe_comp.output.entity),
                   recipe_comp.output.amount);
    // Some basic calculator so that we can make things easier
    ImGui::Separator();
    double expected_cost = GetLedgerCost(GetUniverse(), recipe_comp.input) * expected_production + GetLedgerCost(GetUniverse(), recipe_comp.capitalcost);
    double expected_income = GetUniverse().get<components::Price>(recipe_comp.output.entity) * recipe_comp.output.amount * expected_production;
    double expected_profit = expected_income - expected_cost;
    ImGui::TextFmt("Expected Income: {}", expected_income);
    ImGui::TextFmt("Expected Cost: {}", expected_cost);
    ImGui::TextFmt("Expected Profit: {}", expected_profit);
    ImGui::DragFloat("Amount Produced", &expected_production, 1, 0, 10000000);
}

void SysRecipeViewer::ResetSelection() {
    expected_production = 1;
}
}  // namespace cqsp::client::systems
