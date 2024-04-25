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
#include "common/systems/loading/timezoneloader.h"

#include "common/components/name.h"
#include "common/components/surface.h"

using cqsp::common::systems::loading::TimezoneLoader;
bool TimezoneLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    // Read timezones
    double offset = values["offset"].to_double();
    universe.emplace<components::TimeZone>(entity, offset);
    universe.time_zones[universe.get<components::Identifier>(entity).identifier] = entity;
    return true;
}

void TimezoneLoader::PostLoad(const entt::entity& entity) {}