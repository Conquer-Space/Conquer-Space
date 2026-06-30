/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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
#include "core/loading/modifierloader.h"

#include <spdlog/spdlog.h>

#include <string>

#include "core/components/modifier.h"
#include "core/components/name.h"

namespace cqsp::core::loading {

ModifierLoader::ModifierLoader(Universe& universe) : HjsonLoader(universe) { default_val["amount"] = 0.0; }

bool ModifierLoader::LoadValue(const Hjson::Value& values, Node& node) {
    std::string target_str = values["target"].to_string();
    double amount = values["amount"].to_double();

    auto target = components::ModifierTargetFromString(target_str);
    if (!target) {
        SPDLOG_WARN("Unknown modifier target '{}', skipping modifier", target_str);
        return false;
    }

    node.emplace<components::Modifier>(amount, *target);
    universe.modifiers[node.get<components::Identifier>()] = node;
    return true;
}
}  // namespace cqsp::core::loading
