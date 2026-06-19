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

#include <array>
#include <string>
#include <vector>

#include "client/systems/sysgui.h"
#include "core/components/resource.h"

namespace cqsp::client::systems {
class SysRecipeViewer : public SysUserInterface {
 public:
    explicit SysRecipeViewer(engine::Application& app) : SysUserInterface(app) { search_text.fill(0); }

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    void RecipeViewerRight();
    void RecipeHeader();
    void OutputSection(core::components::Recipe& recipe, const std::vector<std::string>& good_list);
    double LaborSection(core::components::Recipe& recipe, const std::vector<std::string>& job_list);
    void ProfitAnalysis(const core::components::Recipe& recipe, double total_job_cost);
    void ConstructionCostSection(const std::vector<std::string>& good_list);
    void ResetSelection();
    void SaveRecipes();
    void InitializeRecipeFiles();
    void CreateRecipe();
    entt::entity selected_recipe = entt::null;

    std::array<char, 255> search_text;
    float expected_production;
    const ImVec4 id_copy_color = ImVec4(0.921568627f, 0.392156863f, 0.203921569f, 1.f);
    int new_input_good_idx = 0;
    int new_capital_good_idx = 0;
    int new_labor_job_idx = 0;
    int new_output_good_idx = 0;
    int new_construction_good_idx = 0;
    int new_zoning_idx = 0;
};
}  // namespace cqsp::client::systems
