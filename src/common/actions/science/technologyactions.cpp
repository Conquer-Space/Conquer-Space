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
#include "common/actions/science/technologyactions.h"

#include <spdlog/spdlog.h>

#include "common/components/name.h"
#include "common/components/science.h"

namespace cqsp::common::actions {

void ResearchTech(Universe& universe, entt::entity civilization, entt::entity tech) {
    // Ensure it's a tech or something
    auto& tech_progress = universe.get_or_emplace<components::science::TechnologicalProgress>(civilization);
    tech_progress.researched_techs.emplace(tech);

    // Research technology somehow
    auto& tech_comp = universe.get<components::science::Technology>(tech);
    for (const std::string& act : tech_comp.actions) {
        ProcessAction(universe, civilization, act);
    }
}

void ProcessAction(Universe& universe, entt::entity civilization, const std::string& action) {
    // Process the tech
    // Split by the colon
    auto& tech_progress = universe.get_or_emplace<components::science::TechnologicalProgress>(civilization);

    std::string action_name = action.substr(0, action.find(':'));
    std::string outcome_name = action.substr(action.find(':') + 1, action.size());
    if (action_name == "recipe") {
        // Get the text
        // Now load recipe
        // Add to civilization
        tech_progress.researched_recipes.emplace(universe.recipes[outcome_name]);
    } else if (action_name == "mine") {
        // Now load recipe
        // Add to civilization
        tech_progress.researched_mining.emplace(universe.goods[outcome_name]);
    }
}
}  // namespace cqsp::common::actions
