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
#include "core/util/nameutil.h"

#include <fmt/format.h>

#include "core/actions/population/cityinformation.h"
#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/player.h"
#include "core/components/resource.h"
#include "core/components/science.h"
#include "core/components/ships.h"
#include "core/components/surface.h"
#include "core/util/utilnumberdisplay.h"

namespace cqsp::core::util {
namespace components = cqsp::core::components;
std::string GetName(const Universe& universe, entt::entity entity) {
    if (!universe.valid(entity)) {
        return "Null Entity!";
    }
    if (universe.all_of<components::Name>(entity)) {
        return universe.get<components::Name>(entity);
    } else if (universe.all_of<components::Identifier>(entity)) {
        return universe.get<components::Identifier>(entity);
    } else {
        return fmt::format("{}", GetEntityType(universe, entity));
    }
}

std::string GetName(const Universe& universe, components::GoodEntity entity) {
    entt::entity actual_entity = universe.GetGood(entity);
    return GetName(universe, actual_entity);
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
    } else if (universe.any_of<components::Settlement, components::Settlements>(entity)) {
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
}  // namespace cqsp::core::util
