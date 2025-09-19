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
#include "common/loading/technology.h"

#include <spdlog/spdlog.h>

#include "common/actions/science/technologyactions.h"
#include "common/components/name.h"
#include "common/components/science.h"
#include "common/loading/loadutil.h"

namespace cqsp::common::loading {
//TODO(AGM) move to loading namespace
void LoadTechnologies(Universe& universe, Hjson::Value& value) {
    // Load the technologies
    Hjson::Value base;
    base["actions"] = Hjson::Type::Vector;
    base["fields"] = Hjson::Type::Vector;
    base["difficulty"] = 10.;

    for (int i = 0; i < value.size(); i++) {
        Hjson::Value element = Hjson::Merge(base, value[i]);

        entt::entity entity = universe.create();
        if (!loading::LoadInitialValues(universe, entity, element)) {
            // Then kill the loading because you need an identifier
        }

        auto& tech = universe.emplace<components::science::Technology>(entity);
        // Add tech data
        Hjson::Value val = element["actions"];
        for (int i = 0; i < val.size(); i++) {
            tech.actions.push_back(val[i].to_string());
        }

        Hjson::Value fieldlist = element["fields"];
        for (int i = 0; i < fieldlist.size(); i++) {
            entt::entity field_entity = universe.fields[fieldlist[i].to_string()];
            tech.fields.insert(field_entity);
        }

        // Verify if the tags exist
        tech.difficulty = element["difficulty"];

        universe.technologies[universe.get<components::Identifier>(entity)] = entity;
    }
}

}  // namespace cqsp::common::loading
