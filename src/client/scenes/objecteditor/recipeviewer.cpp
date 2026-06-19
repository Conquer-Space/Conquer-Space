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

#include "client/scenes/objecteditor/editorwidgets.h"
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

std::string SecondsToTimeString(int seconds) {
    constexpr int YEAR = 60 * 60 * 24 * 365;
    constexpr int WEEK = 60 * 60 * 24 * 7;
    constexpr int DAY = 60 * 60 * 24;
    constexpr int HOUR = 60 * 60;
    constexpr int MINUTE = 60;
    if (seconds % YEAR == 0) return fmt::format("{} y", seconds / YEAR);
    if (seconds % WEEK == 0) return fmt::format("{} weeks", seconds / WEEK);
    if (seconds % DAY == 0) return fmt::format("{} d", seconds / DAY);
    if (seconds % HOUR == 0) return fmt::format("{} h", seconds / HOUR);
    if (seconds % MINUTE == 0) return fmt::format("{} m", seconds / MINUTE);
    return fmt::format("{} s", seconds);
}
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
    ImGui::BeginChild("recipe_viewer_left", ImVec2(300, -1));
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
    ImGui::BeginChild("recipe_viewer_right", ImVec2(-1, -1));
    RecipeViewerRight();
    ImGui::EndChild();
    ImGui::End();
}

void SysRecipeViewer::DoUpdate(int delta_time) {}

void SysRecipeViewer::RecipeViewerRight() {
    if (selected_recipe == entt::null) {
        ImGui::Text("Good is invalid!");
        return;
    }

    RecipeHeader();

    auto& recipe_comp = GetUniverse().get<components::Recipe>(selected_recipe);
    ImGui::TextFmt("Workers per unit of recipe: {}", recipe_comp.workers.GetSum());

    std::vector<std::string> good_list;
    for (auto& [id, entity] : GetUniverse().goods) good_list.push_back(id);
    std::vector<std::string> job_list;
    for (auto& [id, entity] : GetUniverse().jobs) job_list.push_back(id);

    ImGui::Separator();
    ImGui::Text("Input");
    ResourceVectorSection(GetUniverse(), recipe_comp.input, good_list, new_input_good_idx, "input_table", "Amount",
                          "##new_input_good", "+ Input", id_copy_color);

    ImGui::Separator();
    ImGui::Text("Capital Cost");
    ResourceVectorSection(GetUniverse(), recipe_comp.capitalcost, good_list, new_capital_good_idx, "capital_table",
                          "Amount", "##new_capital_good", "+ Capital", id_copy_color);

    ImGui::Separator();
    OutputSection(recipe_comp, good_list);

    ImGui::Separator();
    double total_job_cost = LaborSection(recipe_comp, job_list);

    ImGui::Separator();
    if (GetUniverse().any_of<FileTag>(selected_recipe)) {
        ImGui::TextFmt("Output File");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("###output_file", &GetUniverse().get<FileTag>(selected_recipe).file);
    }

    ImGui::Separator();
    ProfitAnalysis(recipe_comp, total_job_cost);

    ImGui::Separator();
    ConstructionCostSection(good_list);
}

void SysRecipeViewer::RecipeHeader() {
    ImGui::TextFmt("Name");
    ImGui::SameLine();
    ImGui::InputText("###name", &GetUniverse().get<components::Name>(selected_recipe).name);
    ImGui::TextFmt("Identifier");
    ImGui::SameLine();
    ImGui::InputText("###identifier", &GetUniverse().get<components::Identifier>(selected_recipe).identifier);
    IdentifierTooltipOnItem(GetUniverse(), selected_recipe, id_copy_color);
}

void SysRecipeViewer::OutputSection(components::Recipe& recipe, const std::vector<std::string>& good_list) {
    ImGui::Text("Output");
    ImGui::TextFmt(
        "Output Cost: {}",
        util::NumberToHumanString(GetUniverse().get<components::Price>(recipe.output.entity) * recipe.output.amount));
    if (!good_list.empty()) {
        const std::string& current_output_id =
            GetUniverse().get<components::Identifier>(recipe.output.entity).identifier;
        auto it = std::find(good_list.begin(), good_list.end(), current_output_id);
        if (it != good_list.end()) new_output_good_idx = static_cast<int>(it - good_list.begin());
    }
    ImGui::SetNextItemWidth(180);
    if (GoodCombo("##output_good", new_output_good_idx, good_list) && !good_list.empty()) {
        recipe.output.entity = GetUniverse().good_map[GetUniverse().goods[good_list[new_output_good_idx]]];
    }
    ImGui::SameLine();
    ImGui::InputDouble("###output_amount", &recipe.output.amount);
}

double SysRecipeViewer::LaborSection(components::Recipe& recipe, const std::vector<std::string>& job_list) {
    double total_job_cost = 0;
    int labor_erase = -1;
    if (ImGui::BeginTable("job_cost_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Labor");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Total Cost");
        ImGui::TableSetupColumn("##labor_del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        for (int i = 0; i < static_cast<int>(recipe.workers.workers.size()); i++) {
            auto& [job, workers] = recipe.workers.workers[i];
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
    if (labor_erase >= 0) recipe.workers.workers.erase(recipe.workers.workers.begin() + labor_erase);
    ImGui::SetNextItemWidth(180);
    GoodCombo("##new_labor_job", new_labor_job_idx, job_list);
    ImGui::SameLine();
    if (ImGui::Button("+ Labor") && !job_list.empty()) {
        entt::entity job = GetUniverse().jobs[job_list[new_labor_job_idx]];
        recipe.workers.workers.emplace_back(job, 100u);
    }
    return total_job_cost;
}

void SysRecipeViewer::ProfitAnalysis(const components::Recipe& recipe, double total_job_cost) {
    double expected_cost = GetLedgerCost(GetUniverse(), recipe.input) * expected_production +
                           GetLedgerCost(GetUniverse(), recipe.capitalcost) + total_job_cost;
    double expected_income =
        GetUniverse().get<components::Price>(recipe.output.entity) * recipe.output.amount * expected_production;
    double expected_profit = expected_income - expected_cost;
    ImGui::TextFmt("Expected Income: {}", util::NumberToHumanString(expected_income));
    ImGui::TextFmt("Expected Cost: {}", util::NumberToHumanString(expected_cost));
    ImGui::TextFmt("Job Cost: {}", util::NumberToHumanString(total_job_cost));
    ImGui::TextFmt("Expected Profit: {}", util::NumberToHumanString(expected_profit));
    ImGui::TextFmt("P/L ratio: {}", expected_income / expected_cost);
    ImGui::DragFloat("Amount Produced", &expected_production, 1, 0, 10000000);
}

void SysRecipeViewer::ConstructionCostSection(const std::vector<std::string>& good_list) {
    ImGui::Text("Construction Cost");
    auto& construction_cost = GetUniverse().get<components::ConstructionCost>(selected_recipe);
    ImGui::InputInt("Time (ticks)", &construction_cost.time);
    ResourceVectorSection(GetUniverse(), construction_cost.cost, good_list, new_construction_good_idx,
                          "construction_cost_table", "Amount (per tick)", "##new_construction_good",
                          "+ Construction Cost", id_copy_color);

    ImGui::Text("Zoning Requirements");
    std::vector<std::string> zoning_list;
    for (auto& [id, entity] : GetUniverse().zoning) zoning_list.push_back(id);
    int zoning_erase = -1;
    if (ImGui::BeginTable("zoning_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Zone Type");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("##zoning_del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        for (int z = 0; z < static_cast<int>(construction_cost.zoning.size()); z++) {
            auto& [zone_entity, zone_amount] = construction_cost.zoning[z];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), zone_entity));
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            ImGui::PushID(z);
            ImGui::InputInt("##zone_amount", &zone_amount);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("-")) zoning_erase = z;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (zoning_erase >= 0) construction_cost.zoning.erase(construction_cost.zoning.begin() + zoning_erase);
    if (!zoning_list.empty()) {
        ImGui::SetNextItemWidth(180);
        GoodCombo("##new_zoning_type", new_zoning_idx, zoning_list);
        ImGui::SameLine();
        if (ImGui::Button("+ Zoning")) {
            entt::entity zone_entity = GetUniverse().zoning[zoning_list[new_zoning_idx]];
            construction_cost.zoning.emplace_back(zone_entity, 1);
        }
    }
}

void SysRecipeViewer::ResetSelection() {
    expected_production = 1;
    new_input_good_idx = 0;
    new_capital_good_idx = 0;
    new_labor_job_idx = 0;
    new_output_good_idx = 0;
    new_construction_good_idx = 0;
    new_zoning_idx = 0;
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

            auto& construction_cost = GetUniverse().get<components::ConstructionCost>(recipe);
            int time_seconds = construction_cost.time * components::StarDate::TIME_INCREMENT;
            value["construction"]["time"] = SecondsToTimeString(time_seconds);
            for (auto& [entity, amount] : construction_cost.cost) {
                value["construction"]["cost"][GetUniverse().get<components::Identifier>(entity).identifier] =
                    amount * time_seconds;
            }
            for (auto& [zone_entity, zone_amount] : construction_cost.zoning) {
                value["construction"]["zoning"][GetUniverse().get<components::Identifier>(zone_entity).identifier] =
                    zone_amount;
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
        contents.set_comment_before("# This file is part of Conquer Space\n");
        if (!file.empty()) {
            Hjson::MarshalToFile(contents, file);
        }
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
