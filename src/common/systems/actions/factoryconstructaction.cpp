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

#include <spdlog/spdlog.h>

#include "common/components/resource.h"
#include "common/components/area.h"


using cqsp::common::Universe;
entt::entity cqsp::common::systems::actions::OrderConstructionFactory(
    cqsp::common::Universe& universe, entt::entity city,
    entt::entity recipe, int productivity, entt::entity builder) {
    return entt::entity();
}

entt::entity cqsp::common::systems::actions::CreateFactory(
    Universe& universe, entt::entity city, entt::entity recipe,
    int productivity) {
    namespace cqspc = cqsp::common::components;
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

cqsp::common::components::ResourceLedger
cqsp::common::systems::actions::GetFactoryCost(
    cqsp::common::Universe& universe, entt::entity city,
    entt::entity recipe, int productivity) {
    cqsp::common::components::ResourceLedger ledger;
    ledger[universe.goods["concrete"]] = 1000;
    return ledger;
}

entt::entity cqsp::common::systems::actions::CreateMine(
    cqsp::common::Universe& universe, entt::entity city,
    entt::entity good, int amount) {
    namespace cqspc = cqsp::common::components;
    entt::entity mine = universe.create();
    auto& gen = universe.emplace<cqspc::ResourceGenerator>(mine);
    universe.emplace<cqspc::Mine>(mine);

    gen.emplace(good, amount);
    universe.get<cqspc::Industry>(city).industries.push_back(mine);

    // Add producivity
    universe.emplace<cqspc::FactoryProductivity>(mine, 1.f);

    universe.emplace<cqspc::ResourceStockpile>(mine);
    return mine;
}

cqsp::common::components::ResourceLedger
cqsp::common::systems::actions::GetMineCost(
    cqsp::common::Universe& universe, entt::entity city,
    entt::entity good, int amount) {
    return cqsp::common::components::ResourceLedger();
}
