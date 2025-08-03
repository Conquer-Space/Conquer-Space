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

namespace cqsp::common::util {
namespace components = cqsp::common::components;
std::string GetName(const Universe& universe, entt::entity entity) {
    if (!universe.valid(entity)) {
        return "";
    }
    if (universe.all_of<components::Name>(entity)) {
        return universe.get<components::Name>(entity);
    } else if (universe.all_of<components::Identifier>(entity)) {
        return universe.get<components::Identifier>(entity);
    } else {
        return fmt::format("{}", GetEntityType(universe, entity));
    }
}

/**
 * I feel like this is a very inefficient way of getting the name
 * Perhaps we could just have a component that stores the name, and we
 * can get rid of this complex and honestly unwieldy function.
 */
std::string GetEntityType(const Universe& universe, entt::entity entity) {
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
        auto& generator = universe.get<components::ResourceGenerator>(entity);
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            production += universe.get<components::Name>(it->first).name + ", ";
        }
        // Remove last comma
        if (!production.empty()) {
            production = production.substr(0, production.size() - 2);
        }
        return fmt::format("{} Mine", production);
        */
        return "Mine";
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
    } else if (universe.any_of<components::ships::Ship>(entity)) {
        return "Ship";
    }
    return "Unknown";
}
}  // namespace cqsp::common::util
