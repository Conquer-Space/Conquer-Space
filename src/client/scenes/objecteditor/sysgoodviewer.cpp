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
#include "common/components/market.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/components/tags.h"
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
    ImGui::InputText("##good_viewer_search_text", search_text.data(), search_text.size());
    std::string search_string(search_text.data());
    std::transform(search_string.begin(), search_string.end(), search_string.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    ImGui::BeginChild("good_viewer_scroll");
    for (entt::entity good : goods) {
        std::string name = common::util::GetName(GetUniverse(), good);
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!search_string.empty()) {
            // Then we can check if the text contains it
            if (name_lower.find(search_string) == std::string::npos) {
                continue;
            }
        }
        bool is_selected = good == selected_good;
        // Now also compute that
        if (ImGui::SelectableFmt("{}", &is_selected, name)) {
            selected_good = good;
        }
    }
    ImGui::EndChild();
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
    if (ImGui::IsItemClicked()) {
        // Copy
        ImGui::SetClipboardText(GetUniverse().get<components::Identifier>(selected_good).identifier.c_str());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.921568627, 0.392156863, 0.203921569, 1), "Click to copy identifier");
        ImGui::EndTooltip();
    }
    if (GetUniverse().any_of<components::Matter>(selected_good)) {
        auto& good_comp = GetUniverse().get<components::Matter>(selected_good);
        ImGui::TextFmt("Mass: {} kg", good_comp.mass);
        ImGui::TextFmt("Volume: {} m3", good_comp.volume);
    }
    ImGui::Separator();
    ImGui::TextFmt("Tags");
    if (GetUniverse().any_of<components::Tags>(selected_good)) {
        auto& tag_component = GetUniverse().get<components::Tags>(selected_good);
        for (auto& tag : tag_component.tags) {
            // Now print tags
            ImGui::TextFmt("{}", tag);
        }
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
        RecipeTooltip(recipe);
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
        RecipeTooltip(recipe);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextFmt("{}", recipe_comp.output.amount);
    }
    ImGui::EndTable();
}

void SysGoodViewer::RecipeTooltip(entt::entity recipe) {
    if (ImGui::IsItemClicked()) {
        // Copy
        ImGui::SetClipboardText(GetUniverse().get<components::Identifier>(recipe).identifier.c_str());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.921568627, 0.392156863, 0.203921569, 1), "Click to copy identifier");
        systems::gui::EntityTooltipContent(GetUniverse(), recipe);
        ImGui::EndTooltip();
    }
}
}  // namespace cqsp::client::systems
