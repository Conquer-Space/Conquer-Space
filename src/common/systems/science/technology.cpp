/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "common/systems/science/technology.h"

#include <spdlog/spdlog.h>

#include "common/components/name.h"
#include "common/components/science.h"
#include "common/systems/loading/loadutil.h"

namespace science = cqsp::common::components::science;
using science::Technology;
using science::TechnologicalProgress;
using entt::entity;

namespace cqsp::common::systems::science {
void LoadTechnologies(Universe& universe, Hjson::Value& value) {
    // Load the technologies
    Hjson::Value base;
    base["actions"] = Hjson::Type::Vector;
    base["fields"] = Hjson::Type::Vector;
    base["difficulty"] = 10.;

    for (int i = 0; i < value.size(); i++) {
        Hjson::Value element = Hjson::Merge(base, value[i]);

        entity techentity = universe.create();
        if (!loading::LoadInitialValues(universe, techentity, element)) {
            // Then kill the loading because you need an identifier
        }

        auto& tech = universe.emplace<Technology>(techentity);
        // Add tech data
        Hjson::Value val = element["actions"];
        for (int i = 0; i < val.size(); i++) {
            tech.actions.push_back(val[i].to_string());
        }

        Hjson::Value fieldlist = element["fields"];
        for (int i = 0; i < fieldlist.size(); i++) {
            entity field_entity = universe.fields[fieldlist[i].to_string()];
            tech.fields.insert(field_entity);
        }

        // Verify if the tags exist
        tech.difficulty = element["difficulty"];

        universe.technologies[universe.get<components::Identifier>(techentity)] = techentity;
    }
}

void ResearchTech(Universe& universe, entt::entity civilization, entt::entity tech) {
    // Ensure it's a tech or something
    auto& tech_progress = universe.get_or_emplace<TechnologicalProgress>(civilization);
    tech_progress.researched_techs.emplace(tech);

    // Research technology somehow
    for (const std::string& act : universe.get<Technology>(tech).actions) {
        ProcessAction(universe, civilization, act);
    }
}

void ProcessAction(Universe& universe, entity civilization, const std::string& action) {
    // Process the tech
    // Split by the colon
    auto& tech_progress = universe.get_or_emplace<TechnologicalProgress>(civilization);

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
}  // namespace cqsp::common::systems::science
