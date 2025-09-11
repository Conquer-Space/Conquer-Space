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
#include "client/scenes/objecteditor/sysgoodviewer.h"

#include "client/scenes/universe/interface/systooltips.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/nameutil.h"

namespace cqsp::client::systems {

namespace components = common::components;

void SysGoodViewer::Init() {}

void SysGoodViewer::DoUI(int delta_time) {
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Good Viewer");
    // List out all the stuff
    auto goods = GetUniverse().view<components::Good>();
    ImGui::TextFmt("Goods: {}", goods.size());
    ImGui::BeginChild("Good_viewer_left", ImVec2(300, -1));
    for (entt::entity good : goods) {
        bool is_selected = good == selected_good;
        if (ImGui::SelectableFmt("{}", &is_selected, common::util::GetName(GetUniverse(), good))) {
            selected_good = good;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("good_viewer_right", ImVec2(500, -1));
    GoodViewerRight();
    ImGui::EndChild();
    ImGui::End();
}

void SysGoodViewer::DoUpdate(int delta_time) {}

void SysGoodViewer::GoodViewerRight() {
    if (selected_good == entt::null) {
        ImGui::Text("Good is invalid!");
        return;
    }
    ImGui::TextFmt("Name: {}", common::util::GetName(GetUniverse(), selected_good));
    ImGui::TextFmt("Identifier: {}", GetUniverse().get<components::Identifier>(selected_good).identifier);
    if (GetUniverse().any_of<components::Matter>(selected_good)) {
        auto& good_comp = GetUniverse().get<components::Matter>(selected_good);
        ImGui::TextFmt("Mass: {} kg", good_comp.mass);
        ImGui::TextFmt("Volume: {} m3", good_comp.volume);
    }
    ImGui::Separator();
    ImGui::TextFmt("Tags");
    ImGui::BeginChild("Tags_a");
    if (GetUniverse().any_of<components::Mineral>(selected_good)) {
        ImGui::TextFmt("mineral");
    }
    if (GetUniverse().any_of<components::CapitalGood>(selected_good)) {
        ImGui::TextFmt("capitalgood");
    }
    if (GetUniverse().any_of<components::Price>(selected_good)) {
        ImGui::Separator();
        auto& price_comp = GetUniverse().get<components::Price>(selected_good);
        ImGui::TextFmt("Initial Price: {}", price_comp.price);
    }
    if (GetUniverse().any_of<components::ConsumerGood>(selected_good)) {
        ImGui::Separator();
        auto& consumer = GetUniverse().get<components::ConsumerGood>(selected_good);
        ImGui::TextFmt("Autonomous Consumption: {}", consumer.autonomous_consumption);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Consumption that is independent of disposable income or when income levels are zero.");
            ImGui::EndTooltip();
        }
        ImGui::TextFmt("Marginal Propensity: {}", consumer.marginal_propensity);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Consumption that scales linearly based off amount of surplus income they have.");
            ImGui::EndTooltip();
        }
    }
    // Find all recipes that lead up to this
    ImGui::Separator();
    RecipeTable();
    ImGui::EndChild();
}

void SysGoodViewer::RecipeTable() {
    OutputRecipeTable();
    InputRecipeTable();
}

void SysGoodViewer::InputRecipeTable() {
    auto recipes = GetUniverse().view<components::Recipe>();
    ImGui::Text("Consumed By");
    // Also see what it leads into
    if (!ImGui::BeginTable("input_recipe_table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        return;
    }
    ImGui::TableSetupColumn("Recipe");
    ImGui::TableSetupColumn("Amount");
    ImGui::TableHeadersRow();
    for (entt::entity recipe : recipes) {
        auto& recipe_comp = GetUniverse().get<components::Recipe>(recipe);
        if (!recipe_comp.input.contains(selected_good)) {
            continue;
        }
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), recipe));
        ImGui::TableSetColumnIndex(1);
        ImGui::TextFmt("{}", recipe_comp.input[selected_good]);
    }
    ImGui::EndTable();
}

void SysGoodViewer::OutputRecipeTable() {
    auto recipes = GetUniverse().view<components::Recipe>();
    ImGui::Text("Created By");

    if (!ImGui::BeginTable("recipe_table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        return;
    }
    ImGui::TableSetupColumn("Recipe");
    ImGui::TableSetupColumn("Amount");
    ImGui::TableHeadersRow();
    for (entt::entity recipe : recipes) {
        auto& recipe_comp = GetUniverse().get<components::Recipe>(recipe);
        if (recipe_comp.output.entity != selected_good) {
            continue;
        }
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), recipe));
        ImGui::TableSetColumnIndex(1);
        ImGui::TextFmt("{}", recipe_comp.output.amount);
    }
    ImGui::EndTable();
}
}  // namespace cqsp::client::systems
