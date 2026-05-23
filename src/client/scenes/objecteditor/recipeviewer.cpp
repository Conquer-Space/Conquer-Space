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

#include <imgui_stdlib.h>

#include <filesystem>

#include "client/scenes/universe/interface/ledgertable.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "core/components/labor.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/resource.h"
#include "core/util/nameutil.h"
#include "core/util/paths.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/asset/textasset.h"
#include "engine/asset/vfs/nativevfs.h"

namespace cqsp::client::systems {
namespace components = core::components;

namespace {
struct FileTag {
    std::string file;
};
}  // namespace

void SysRecipeViewer::Init() { InitializeRecipeFiles(); }

void SysRecipeViewer::DoUI(int delta_time) {
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Recipe Viewer");
    // List out all the stuff
    auto recipes = GetUniverse().view<components::Recipe>();
    ImGui::TextFmt("Recipes: {}", recipes.size());
    ImGui::BeginChild("recipe_viewer_left", ImVec2(300, 700));
    ImGui::InputText("##recipe_viewer_search_text", search_text.data(), search_text.size());
    std::string search_string(search_text.data());
    std::transform(search_string.begin(), search_string.end(), search_string.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    ImGui::BeginChild("recipe_viewer_scroll");
    for (entt::entity recipe : recipes) {
        bool is_selected = recipe == selected_recipe;
        std::string name = core::util::GetName(GetUniverse(), recipe);
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (!search_string.empty()) {
            // Then we can check if the text contains it
            if (name_lower.find(search_string) == std::string::npos) {
                continue;
            }
        }
        // Now check if the string is in stuff
        if (ImGui::SelectableFmt("{}", &is_selected, name)) {
            selected_recipe = recipe;
            ResetSelection();
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("recipe_viewer_right", ImVec2(400, 700));
    RecipeViewerRight();
    ImGui::EndChild();
    ImGui::End();
}

void SysRecipeViewer::DoUpdate(int delta_time) {}

namespace {
double GetLedgerCost(core::Universe& universe, const components::ResourceVector& ledger) {
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
    ImGui::TextFmt("Name: {}", core::util::GetName(GetUniverse(), selected_recipe));
    ImGui::TextFmt("Identifier: {}", GetUniverse().get<components::Identifier>(selected_recipe).identifier);
    if (ImGui::IsItemClicked()) {
        // Then copy
        ImGui::SetClipboardText(GetUniverse().get<components::Identifier>(selected_recipe).identifier.c_str());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(id_copy_color, "Click to copy identifier");
        ImGui::EndTooltip();
    }
    // Get inputs and outputs
    auto& recipe_comp = GetUniverse().get<components::Recipe>(selected_recipe);
    ImGui::TextFmt("Workers per unit of recipe: {}", recipe_comp.workers.GetSum());
    ImGui::Text("Input");
    ImGui::TextFmt("Input Default Cost: {}", GetLedgerCost(GetUniverse(), recipe_comp.input));
    ResourceMapTable(GetUniverse(), recipe_comp.input, "input_table");
    ImGui::Separator();
    ImGui::Text("Capital Cost");
    ImGui::TextFmt("Capital Default Cost: {}",
                   util::NumberToHumanString(GetLedgerCost(GetUniverse(), recipe_comp.capitalcost)));
    ResourceMapTable(GetUniverse(), recipe_comp.capitalcost, "capital_table");
    ImGui::Separator();
    ImGui::Text("Output");
    ImGui::TextFmt("Output Cost: {}",
                   util::NumberToHumanString(GetUniverse().get<components::Price>(recipe_comp.output.entity) *
                                             recipe_comp.output.amount));
    ImGui::TextFmt("{}, {}", core::util::GetName(GetUniverse(), recipe_comp.output.entity),
                   util::NumberToHumanString(recipe_comp.output.amount));
    if (ImGui::IsItemClicked()) {
        // Then copy
        ImGui::SetClipboardText(
            GetUniverse().get<components::Identifier>(recipe_comp.output.entity).identifier.c_str());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(id_copy_color, "Click to copy identifier");
        ImGui::EndTooltip();
    }
    ImGui::Separator();
    double total_job_cost = 0;
    if (ImGui::BeginTable("Job cost table", 3)) {
        // Now go through all the jobs
        ImGui::TableSetupColumn("Labor");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Total Cost");
        ImGui::TableHeadersRow();
        for (const auto& [job, workers] : recipe_comp.workers.workers) {
            auto& labor = GetUniverse().get<components::Labor>(job);
            // Now get the price and all that
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), job));
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", workers);
            ImGui::TableSetColumnIndex(2);
            ImGui::TextFmt("{}", GetUniverse().get<components::Price>(labor.good).price * workers);
            total_job_cost += GetUniverse().get<components::Price>(labor.good).price * workers;
        }
        ImGui::EndTable();
    }
    // Some basic calculator so that we can make things easier
    ImGui::Separator();
    double expected_cost = GetLedgerCost(GetUniverse(), recipe_comp.input) * expected_production +
                           GetLedgerCost(GetUniverse(), recipe_comp.capitalcost) + total_job_cost;
    double expected_income = GetUniverse().get<components::Price>(recipe_comp.output.entity) *
                             recipe_comp.output.amount * expected_production;
    double expected_profit = expected_income - expected_cost;
    ImGui::TextFmt("Expected Income: {}", util::NumberToHumanString(expected_income));
    ImGui::TextFmt("Expected Cost: {}", util::NumberToHumanString(expected_cost));
    ImGui::TextFmt("Job Cost: {}", util::NumberToHumanString(total_job_cost));
    ImGui::TextFmt("Expected Profit: {}", util::NumberToHumanString(expected_profit));
    ImGui::TextFmt("P/L ratio: {}", expected_income / expected_cost);
    ImGui::DragFloat("Amount Produced", &expected_production, 1, 0, 10000000);
}

void SysRecipeViewer::ResetSelection() { expected_production = 1; }

void SysRecipeViewer::InitializeRecipeFiles() {
    auto& asset_manager = GetApp().GetAssetManager();
    auto* recipes = asset_manager.GetAsset<asset::HjsonAsset>("recipes");

    std::filesystem::path data_path(cqsp::core::util::GetCqspDataPath());
    data_path = data_path / recipes->path;
    // List directory the normal way
    for (const auto& entry : std::filesystem::recursive_directory_iterator(data_path)) {
        auto output = Hjson::UnmarshalFromFile(entry.path().string());
        // Loop through them and then process
        for (int i = 0; i < output.size(); i++) {
            const Hjson::Value& val = output[i];
            std::string identifier = val["identifier"].to_string();
            if (GetUniverse().recipes.contains(identifier)) {
                GetUniverse().emplace_or_replace<FileTag>(GetUniverse().recipes[identifier], entry.path().string());
            } else {
                SPDLOG_INFO("Invalid identifier {}", identifier);
            }
        }
    }
}

void SysRecipeViewer::SaveRecipes() {}
}  // namespace cqsp::client::systems
