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
#include "common/util/nameutil.h"

#include <fmt/format.h>

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/resource.h"
#include "common/components/science.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/systems/population/cityinformation.h"
#include "common/util/utilnumberdisplay.h"

namespace components = cqsp::common::components;

using components::Name;
using components::Identifier;
using entt::entity;

namespace cqsp::common::util {
std::string GetName(const Universe& universe, entity entity) {
    if (!universe.valid(entity)) {
        return "";
    }
    if (universe.all_of<Name>(entity)) {
        return universe.get<Name>(entity);
    } else if (universe.all_of<Identifier>(entity)) {
        return universe.get<Identifier>(entity);
    } else {
        return fmt::format("{}", GetEntityType(universe, entity));
    }
}

std::string GetEntityType(const Universe& universe, entity entity) {
    // Then get type of entity
    if (entity == entt::null) {
        return "Null Entity";
    }
    if (universe.all_of<components::bodies::Star>(entity)) {
        return "Star";
    } else if (universe.all_of<components::bodies::Planet>(entity)) {
        return "Planet";
    } else if (universe.any_of<components::Settlement, components::Habitation>(entity)) {
        return "City";
    } else if (universe.any_of<components::Production>(entity)) {
        auto& generator = universe.get<components::Production>(entity);
        return fmt::format("{} Factory", GetName(universe, generator.recipe));
    } else if (universe.any_of<components::Mine>(entity)) {
        /*
        std::string production = "";
        auto& generator = universe.get<cqspc::ResourceGenerator>(entity);
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            production += universe.get<cqspc::Name>(it->first).name + ", ";
        }
        // Remove last comma
        if (!production.empty()) {
            production = production.substr(0, production.size() - 2);
        }
        return fmt::format("{} Mine", production);
        */
    } else if (universe.any_of<components::Player>(entity)) {
        return "Player";
    } else if (universe.any_of<components::Country>(entity)) {
        return "Country";
    } else if (universe.any_of<components::Province>(entity)) {
        return "Province";
    } else if (universe.any_of<components::Organization>(entity)) {
        return "Organization";
    } else if (universe.any_of<components::science::Lab>(entity)) {
        return "Science Lab";
    } else if (universe.any_of<components::Commercial>(entity)) {
        return "Commercial";
    }
    return "Unknown";
}
}  // namespace cqsp::common::util
