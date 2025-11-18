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

#include "common/actions/population/cityinformation.h"
#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/infrastructure.h"
#include "common/components/market.h"
#include "common/components/name.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/resource.h"
#include "common/components/science.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/util/utilnumberdisplay.h"

namespace cqsp::common::util {
namespace components = cqsp::common::components;
std::string GetName(Node& node) {
    if (!node.valid()) {
        return "";
    }
    if (node.all_of<components::Name>()) {
        return node.get<components::Name>();
    } else if (node.all_of<components::Identifier>()) {
        return node.get<components::Identifier>();
    } else {
        return fmt::format("{}", GetEntityType(node));
    }
}
std::string GetName(const Universe& universe, entt::entity entity) { 
    Node node(universe, entity);
    return GetName(node);
}

/**
 * I feel like this is a very inefficient way of getting the name
 * Perhaps we could just have a component that stores the name, and we
 * can get rid of this complex and honestly unwieldy function.
 */
std::string GetEntityType(Node& node) {
    // Then get type of entity
    if (node == entt::null) {
        return "Null Entity";
    }
    if (node.all_of<components::bodies::Star>()) {
        return "Star";
    } else if (node.all_of<components::bodies::Planet>()) {
        return "Planet";
    } else if (node.any_of<components::Settlement, components::Habitation>()) {
        return "City";
    } else if (node.any_of<components::Production>()) {
        auto& generator = node.get<components::Production>();
        Node recipe_node = Node(node, generator.recipe);
        return fmt::format("{} Factory", GetName(recipe_node));
    } else if (node.any_of<components::Mine>()) {
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
    } else if (node.any_of<components::Player>()) {
        return "Player";
    } else if (node.any_of<components::Country>()) {
        return "Country";
    } else if (node.any_of<components::Province>()) {
        return "Province";
    } else if (node.any_of<components::Organization>()) {
        return "Organization";
    } else if (node.any_of<components::science::Lab>()) {
        return "Science Lab";
    } else if (node.any_of<components::Commercial>()) {
        return "Commercial";
    } else if (node.any_of<components::ships::Ship>()) {
        return "Ship";
    }
    return "Unknown";
}
std::string GetEntityType(const Universe& universe, entt::entity entity) {
    Node node(universe, entity);
    return GetEntityType(node);
}
}  // namespace cqsp::common::util
