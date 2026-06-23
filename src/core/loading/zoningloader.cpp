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
#include "core/loading/zoningloader.h"

#include "core/components/area.h"
#include "core/components/name.h"

namespace cqsp::core::loading {
bool ZoningLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Get identifier and add to map
    node.emplace<components::ZoningType>();
    universe.zoning[node.get<components::Identifier>()] = node;
    return true;
}

void ZoningLoader::PostLoad(const Node& node) {}
}  // namespace cqsp::core::loading
