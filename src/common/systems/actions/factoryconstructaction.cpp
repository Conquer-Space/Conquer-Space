/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "common/systems/actions/factoryconstructaction.h"

#include "common/components/resource.h"
#include "common/components/area.h"

#include <spdlog/spdlog.h>

using conquerspace::common::components::Universe;
entt::entity conquerspace::common::systems::actions::OrderConstructionFactory(
    conquerspace::common::components::Universe& universe, entt::entity city,
    entt::entity recipe, int productivity, entt::entity builder) {
    return entt::entity();
}
entt::entity conquerspace::common::systems::actions::CreateFactory(
    Universe& universe, entt::entity city, entt::entity recipe,
    int productivity) {
    namespace cqspc = conquerspace::common::components;
    // Make the factory
    entt::entity factory = universe.create();
    auto& factory_converter = universe.emplace<cqspc::ResourceConverter>(factory);
    universe.emplace<cqspc::Factory>(factory);

    // Add producivity
    auto& prod = universe.emplace<cqspc::FactoryProductivity>(factory);
    prod.productivity = productivity;

    universe.emplace<cqspc::ResourceStockpile>(factory);

    // Add recipes and stuff
    factory_converter.recipe = recipe;
    universe.get<cqspc::Industry>(city).industries.push_back(factory);
    return factory;
}

conquerspace::common::components::ResourceLedger
conquerspace::common::systems::actions::GetFactoryCost(
    conquerspace::common::components::Universe& universe, entt::entity city,
    entt::entity recipe, int productivity) {
    conquerspace::common::components::ResourceLedger ledger;
    ledger[universe.goods["concrete"]] = 1000;
    return ledger;
}
