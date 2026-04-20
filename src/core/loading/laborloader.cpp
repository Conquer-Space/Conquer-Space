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
#include "core/loading/laborloader.h"

#include <spdlog/spdlog.h>

#include "core/components/labor.h"
#include "core/components/name.h"

namespace cqsp::core::loading {
bool LaborLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Now just load the good
    // This expects the goods to have been loaded too lol
    const std::string& good_name = values["good"];
    node.emplace<components::Labor>(universe.goods[good_name]);

    if (!values["tags"].empty()) {
        const auto& tags = values["tags"];
        for (int i = 0; i < tags.size(); i++) {
            const std::string& tag = tags[i].to_string();
            if (tag == "default") {
                // Then we should set the default job???
                if (universe.default_job == entt::null) {
                    universe.default_job = node;
                } else {
                    SPDLOG_WARN(
                        "Job {} is the currently default job, which conflicts with {}. It will remain as that, we will "
                        "not be overwriting it",
                        universe.get<components::Identifier>(universe.default_job).identifier,
                        node.get<components::Identifier>().identifier);
                }
            }
        }
    }
    return true;
}

void LaborLoader::PostLoad(const Node& node) {}
}  // namespace cqsp::core::loading
