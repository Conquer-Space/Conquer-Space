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
    ImGui::SameLine();
    if (ImGui::Button("Save Recipes")) {
        SaveRecipes();
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Recipe")) {
        CreateRecipe();
    }
    ImGui::BeginChild("recipe_viewer_left", ImVec2(300, 700));
    ImGui::InputText("##recipe_viewer_search_text", search_text.data(), search_text.size());
    std::string search_string(search_text.data());
    std::transform(search_string.begin(), search_string.end(), search_string.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    ImGui::BeginChild("recipe_viewer_scroll");
    for (entt::entity recipe : recipes) {
        bool is_selected = recipe == selected_recipe;
        std::string name = core::util::GetName(GetUniverse(), recipe);
        std::string identifier_lower = GetUniverse().get<components::Identifier>(recipe).identifier;
        std::transform(identifier_lower.begin(), identifier_lower.end(), identifier_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if ((!search_string.empty() && name_lower.find(search_string) == std::string::npos) &&
            (!identifier_lower.empty() && identifier_lower.find(search_string) == std::string::npos)) {
            // Then we can check if the text contains it
            continue;
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

namespace {
bool GoodCombo(const char* label, int& selected_idx, const std::vector<std::string>& list) {
    if (list.empty()) return false;
    if (selected_idx >= static_cast<int>(list.size())) selected_idx = 0;
    bool changed = false;
    if (ImGui::BeginCombo(label, list[selected_idx].c_str())) {
        static char search_buf[128] = {};
        if (ImGui::IsWindowAppearing()) {
            memset(search_buf, 0, sizeof(search_buf));
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##combo_search", search_buf, sizeof(search_buf));
        std::string search_lower(search_buf);
        std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        for (int i = 0; i < static_cast<int>(list.size()); i++) {
            if (!search_lower.empty()) {
                std::string item_lower = list[i];
                std::transform(item_lower.begin(), item_lower.end(), item_lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                if (item_lower.find(search_lower) == std::string::npos) continue;
            }
            bool is_selected = i == selected_idx;
            if (ImGui::Selectable(list[i].c_str(), is_selected)) {
                selected_idx = i;
                changed = true;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}
}  // namespace

void SysRecipeViewer::RecipeViewerRight() {
    if (selected_recipe == entt::null) {
        ImGui::Text("Good is invalid!");
        return;
    }
    ImGui::TextFmt("Name");
    ImGui::SameLine();
    ImGui::InputText("###name", &GetUniverse().get<components::Name>(selected_recipe).name);
    ImGui::TextFmt("Identifier");
    ImGui::SameLine();
    ImGui::InputText("###identifier", &GetUniverse().get<components::Identifier>(selected_recipe).identifier);
    if (ImGui::IsItemClicked()) {
        ImGui::SetClipboardText(GetUniverse().get<components::Identifier>(selected_recipe).identifier.c_str());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(id_copy_color, "Click to copy identifier");
        ImGui::EndTooltip();
    }

    auto& recipe_comp = GetUniverse().get<components::Recipe>(selected_recipe);
    ImGui::TextFmt("Workers per unit of recipe: {}", recipe_comp.workers.GetSum());

    // Build goods and jobs lists once per frame for the combos
    std::vector<std::string> good_list;
    for (auto& [id, entity] : GetUniverse().goods) good_list.push_back(id);
    std::vector<std::string> job_list;
    for (auto& [id, entity] : GetUniverse().jobs) job_list.push_back(id);

    ImGui::Separator();
    ImGui::Text("Input");
    ImGui::TextFmt("Input Default Cost: {}", GetLedgerCost(GetUniverse(), recipe_comp.input));
    int input_erase = -1;
    if (ImGui::BeginTable("input_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("##input_del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        int i = 0;
        for (auto& [entity, amount] : recipe_comp.input) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), entity));
            if (ImGui::IsItemClicked()) {
                ImGui::SetClipboardText(GetUniverse().get<components::Identifier>(entity).identifier.c_str());
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextColored(id_copy_color, "Click to copy identifier");
                ImGui::EndTooltip();
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            ImGui::PushID(i);
            ImGui::InputDouble("##input_amount", &amount);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("-")) input_erase = i;
            ImGui::PopID();
            i++;
        }
        ImGui::EndTable();
    }
    if (input_erase >= 0) recipe_comp.input.erase(recipe_comp.input.begin() + input_erase);
    ImGui::SetNextItemWidth(180);
    GoodCombo("##new_input_good", new_input_good_idx, good_list);
    ImGui::SameLine();
    if (ImGui::Button("+ Input") && !good_list.empty()) {
        core::components::GoodEntity ge = GetUniverse().good_map[GetUniverse().goods[good_list[new_input_good_idx]]];
        recipe_comp.input.push_back({ge, 1.0});
        recipe_comp.input.Finalize();
    }

    ImGui::Separator();
    ImGui::Text("Capital Cost");
    ImGui::TextFmt("Capital Default Cost: {}",
                   util::NumberToHumanString(GetLedgerCost(GetUniverse(), recipe_comp.capitalcost)));
    int capital_erase = -1;
    if (ImGui::BeginTable("capital_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("##capital_del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        int j = 0;
        for (auto& [entity, amount] : recipe_comp.capitalcost) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), entity));
            if (ImGui::IsItemClicked()) {
                ImGui::SetClipboardText(GetUniverse().get<components::Identifier>(entity).identifier.c_str());
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextColored(id_copy_color, "Click to copy identifier");
                ImGui::EndTooltip();
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            ImGui::PushID(j);
            ImGui::InputDouble("##capital_amount", &amount);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("-")) capital_erase = j;
            ImGui::PopID();
            j++;
        }
        ImGui::EndTable();
    }
    if (capital_erase >= 0) recipe_comp.capitalcost.erase(recipe_comp.capitalcost.begin() + capital_erase);
    ImGui::SetNextItemWidth(180);
    GoodCombo("##new_capital_good", new_capital_good_idx, good_list);
    ImGui::SameLine();
    if (ImGui::Button("+ Capital") && !good_list.empty()) {
        core::components::GoodEntity ge = GetUniverse().good_map[GetUniverse().goods[good_list[new_capital_good_idx]]];
        recipe_comp.capitalcost.push_back({ge, 1.0});
        recipe_comp.capitalcost.Finalize();
    }

    ImGui::Separator();
    ImGui::Text("Output");
    ImGui::TextFmt("Output Cost: {}",
                   util::NumberToHumanString(GetUniverse().get<components::Price>(recipe_comp.output.entity) *
                                             recipe_comp.output.amount));
    // Sync combo index to the current output entity
    if (!good_list.empty()) {
        const std::string& current_output_id =
            GetUniverse().get<components::Identifier>(recipe_comp.output.entity).identifier;
        auto it = std::find(good_list.begin(), good_list.end(), current_output_id);
        if (it != good_list.end()) new_output_good_idx = static_cast<int>(it - good_list.begin());
    }
    ImGui::SetNextItemWidth(180);
    if (GoodCombo("##output_good", new_output_good_idx, good_list) && !good_list.empty()) {
        recipe_comp.output.entity = GetUniverse().good_map[GetUniverse().goods[good_list[new_output_good_idx]]];
    }
    ImGui::SameLine();
    ImGui::InputDouble("###output_amount", &recipe_comp.output.amount);

    ImGui::Separator();
    double total_job_cost = 0;
    int labor_erase = -1;
    if (ImGui::BeginTable("job_cost_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Labor");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Total Cost");
        ImGui::TableSetupColumn("##labor_del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        for (int i = 0; i < static_cast<int>(recipe_comp.workers.workers.size()); i++) {
            auto& [job, workers] = recipe_comp.workers.workers[i];
            auto& labor = GetUniverse().get<components::Labor>(job);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), job));
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            ImGui::PushID(i);
            int workers_int = static_cast<int>(workers);
            if (ImGui::InputInt("##workers", &workers_int)) {
                if (workers_int < 0) workers_int = 0;
                workers = static_cast<uint32_t>(workers_int);
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::TextFmt("{}", GetUniverse().get<components::Price>(labor.good).price * workers);
            total_job_cost += GetUniverse().get<components::Price>(labor.good).price * workers;
            ImGui::TableSetColumnIndex(3);
            if (ImGui::SmallButton("-")) labor_erase = i;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (labor_erase >= 0) recipe_comp.workers.workers.erase(recipe_comp.workers.workers.begin() + labor_erase);
    ImGui::SetNextItemWidth(180);
    GoodCombo("##new_labor_job", new_labor_job_idx, job_list);
    ImGui::SameLine();
    if (ImGui::Button("+ Labor") && !job_list.empty()) {
        entt::entity job = GetUniverse().jobs[job_list[new_labor_job_idx]];
        recipe_comp.workers.workers.emplace_back(job, 100u);
    }

    ImGui::Separator();
    if (GetUniverse().any_of<FileTag>(selected_recipe)) {
        ImGui::TextFmt("Output File");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("###output_file", &GetUniverse().get<FileTag>(selected_recipe).file);
    }

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

void SysRecipeViewer::ResetSelection() {
    expected_production = 1;
    new_input_good_idx = 0;
    new_capital_good_idx = 0;
    new_labor_job_idx = 0;
    new_output_good_idx = 0;
}

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

void SysRecipeViewer::SaveRecipes() {
    auto& asset_manager = GetApp().GetAssetManager();
    auto* recipes = asset_manager.GetAsset<asset::HjsonAsset>("recipes");

    std::filesystem::path data_path(cqsp::core::util::GetCqspDataPath());
    data_path = data_path / recipes->path;

    std::map<std::string, std::vector<entt::entity>> recipe_to_file_map;
    for (auto&& [entity, file, recipe] : GetUniverse().view<FileTag, components::Recipe>().each()) {
        recipe_to_file_map[file.file].push_back(entity);
    }

    for (auto& [file, list] : recipe_to_file_map) {
        Hjson::Value contents;
        std::sort(list.begin(), list.end(), [&](entt::entity left, entt::entity right) {
            return GetUniverse().get<components::Identifier>(left).identifier <
                   GetUniverse().get<components::Identifier>(right).identifier;
        });
        for (entt::entity recipe : list) {
            Hjson::Value value;
            value["identifier"] = GetUniverse().get<components::Identifier>(recipe).identifier;
            value["name"] = GetUniverse().get<components::Name>(recipe).name;
            if (GetUniverse().all_of<components::Description>(recipe)) {
                value["desc"] = GetUniverse().get<components::Description>(recipe).description;
            }
            auto& recipe_comp = GetUniverse().get<components::Recipe>(recipe);

            for (auto& [entity, amount] : recipe_comp.input) {
                value["input"][GetUniverse().get<components::Identifier>(entity).identifier] = amount;
            }

            value["output"][GetUniverse().get<components::Identifier>(recipe_comp.output.entity).identifier] =
                recipe_comp.output.amount;

            bool has_capital = !recipe_comp.capitalcost.empty();
            bool has_labor = !recipe_comp.workers.workers.empty();
            bool has_recipe_cost = GetUniverse().all_of<components::RecipeCost>(recipe);
            if (has_capital || has_labor || has_recipe_cost) {
                if (has_capital) {
                    for (auto& [entity, amount] : recipe_comp.capitalcost) {
                        value["cost"]["capital"][GetUniverse().get<components::Identifier>(entity).identifier] = amount;
                    }
                }
                if (has_labor) {
                    for (auto& [job, workers] : recipe_comp.workers.workers) {
                        value["cost"]["labor"][GetUniverse().get<components::Identifier>(job).identifier] =
                            static_cast<int>(workers);
                    }
                }
                if (has_recipe_cost) {
                    auto& recipe_cost = GetUniverse().get<components::RecipeCost>(recipe);
                    for (auto& [entity, amount] : recipe_cost.fixed) {
                        value["cost"]["fixed"][GetUniverse().get<components::Identifier>(entity).identifier] = amount;
                    }
                    for (auto& [entity, amount] : recipe_cost.scaling) {
                        value["cost"]["scaling"][GetUniverse().get<components::Identifier>(entity).identifier] = amount;
                    }
                }
            }

            switch (recipe_comp.type) {
                case components::ProductionType::mine:
                    value["tags"].push_back("raw");
                    break;
                case components::ProductionType::service:
                    value["tags"].push_back("service");
                    break;
                case components::ProductionType::factory:
                    value["tags"].push_back("factory");
                    break;
            }

            contents.push_back(value);
        }
        Hjson::MarshalToFile(contents, file);
    }
}

void SysRecipeViewer::CreateRecipe() {
    entt::entity recipe = GetUniverse().create();
    components::Recipe& recipe_comp = GetUniverse().emplace<components::Recipe>(recipe);
    GetUniverse().emplace<components::Identifier>(recipe, "new_recipe");
    GetUniverse().emplace<components::Name>(recipe, "New Recipe");
    GetUniverse().emplace<FileTag>(recipe, "");
    // Default output to the first available good
    if (!GetUniverse().goods.empty()) {
        entt::entity first_good = GetUniverse().goods.begin()->second;
        recipe_comp.output.entity = GetUniverse().good_map[first_good];
    }
    recipe_comp.output.amount = 1.0;
    recipe_comp.type = components::ProductionType::factory;
    GetUniverse().recipes["new_recipe"] = recipe;
    selected_recipe = recipe;
}
}  // namespace cqsp::client::systems
