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
#include "common/loading/timezoneloader.h"

#include "common/components/name.h"
#include "common/components/surface.h"

namespace cqsp::common::loading {
bool TimezoneLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Read timezones
    double offset = values["offset"].to_double();
    node.emplace<components::TimeZone>(offset);
    universe.time_zones[node.get<components::Identifier>().identifier] = node;
    return true;
}

void TimezoneLoader::PostLoad(const Node& node) {}
}  // namespace cqsp::common::loading
