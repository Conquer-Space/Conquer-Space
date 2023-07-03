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

namespace cqsp::common::util {
std::string GetName(const Universe& universe, entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    if (universe.all_of<cqspc::Name>(entity)) {
        return universe.get<cqspc::Name>(entity);
    } else if (universe.all_of<cqspc::Identifier>(entity)) {
        return universe.get<cqspc::Identifier>(entity);
    } else {
        return fmt::format("{}", GetEntityType(universe, entity));
    }
}

std::string GetEntityType(const cqsp::common::Universe& universe, entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    // Then get type of entity
    if (entity == entt::null) {
        return "Null Entity";
    }
    if (universe.all_of<cqspc::bodies::Star>(entity)) {
        return "Star";
    } else if (universe.all_of<cqspc::bodies::Planet>(entity)) {
        return "Planet";
    } else if (universe.any_of<cqspc::Settlement, cqspc::Habitation>(entity)) {
        return "City";
    } else if (universe.any_of<cqspc::Production>(entity)) {
        auto& generator = universe.get<cqspc::Production>(entity);
        return fmt::format("{} Factory", GetName(universe, generator.recipe));
    } else if (universe.any_of<cqspc::Mine>(entity)) {
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
    } else if (universe.any_of<cqspc::Player>(entity)) {
        return "Player";
    } else if (universe.any_of<cqspc::Country>(entity)) {
        return "Country";
    } else if (universe.any_of<cqspc::Province>(entity)) {
        return "Province";
    } else if (universe.any_of<cqspc::Organization>(entity)) {
        return "Organization";
    } else if (universe.any_of<cqspc::science::Lab>(entity)) {
        return "Science Lab";
    } else if (universe.any_of<cqspc::Commercial>(entity)) {
        return "Commercial";
    }
    return "Unknown";
}
}  // namespace cqsp::common::util
