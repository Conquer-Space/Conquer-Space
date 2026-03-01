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
#include "core/loading/projectloader.h"

#include "core/components/projects.h"
#include "core/loading/loadutil.h"

namespace cqsp::core::loading {
bool ProjectLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Now we load a project or something
    const Hjson::Value& construction = values["construction"];
    auto& project_template = node.emplace<components::ProjectTemplate>();
    bool time_correct;
    double time = ReadUnit(construction["time"].to_string(), components::types::UnitType::Time, &time_correct);
    if (!time_correct) {
        time = 100;
    }
    project_template.max_progress = time;
    project_template.cost = HjsonToVector(universe, construction["cost"]);
    return true;
}

void ProjectLoader::PostLoad(const Node& node) {}
}  // namespace cqsp::core::loading
