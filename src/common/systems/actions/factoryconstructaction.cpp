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
#include <spdlog/spdlog.h>

#include <map>
#include <limits>

#include "common/components/resource.h"
#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/systems/economy/markethelpers.h"
#include "common/systems/actions/factoryconstructaction.h"

using cqsp::common::Universe;
entt::entity cqsp::common::systems::actions::OrderConstructionFactory(cqsp::common::Universe& universe,
    entt::entity city, entt::entity market, entt::entity recipe, int productivity, entt::entity builder) {
    entt::entity factory = common::systems::actions::CreateFactory(
        universe, city, recipe, productivity);
    if (factory == entt::null) {
        return entt::null;
    }
    cqsp::common::systems::economy::AddParticipant(universe, market, factory);
    auto cost = common::systems::actions::GetFactoryCost(universe, city, recipe, productivity);

    // Buy the goods on the market
    common::systems::economy::PurchaseGood(universe, builder, cost);
    return factory;
}

entt::entity cqsp::common::systems::actions::CreateFactory(Universe& universe, entt::entity city,
    entt::entity recipe, int productivity) {
    namespace cqspc = cqsp::common::components;
    // Make the factory
    if (!universe.any_of<cqspc::IndustrialZone>(city)) {
        SPDLOG_WARN("City {} has no industry", city);
        return entt::null;
    }
    entt::entity factory = universe.create();
    //auto& factory_converter = universe.emplace<cqspc::ResourceConverter>(factory);
    auto& production = universe.emplace<cqspc::Production>(factory);
    // Add recipes and stuff
    production.recipe = recipe;
    universe.get<cqspc::IndustrialZone>(city).industries.push_back(factory);

    // Add capacity
    // Add producivity
    auto& prod = universe.emplace<cqspc::IndustrySize>(factory);
    prod.size = productivity;
    prod.utilization = 1;

    switch (universe.get<cqspc::Recipe>(recipe).type) {
        case cqspc::mine:
            universe.emplace<cqspc::Mine>(factory);
            break;
        case cqspc::service:
            universe.emplace<cqspc::Service>(factory);
            break;
        default:
            universe.emplace<cqspc::Factory>(factory);
    }

    auto& employer = universe.emplace<cqspc::Employer>(factory);
    employer.population_fufilled = 1000000;
    employer.population_needed = 1000000;
    employer.segment = entt::null;
    return factory;
}

cqsp::common::components::ResourceLedger
cqsp::common::systems::actions::GetFactoryCost(cqsp::common::Universe& universe, entt::entity city,
    entt::entity recipe, int capacity) {
    cqsp::common::components::ResourceLedger ledger;
    // Get the recipe and things
    if (universe.any_of<components::RecipeCost>(recipe)) {
        auto& cost = universe.get<components::RecipeCost>(recipe);
        ledger.MultiplyAdd(cost.scaling, capacity);
        ledger += cost.fixed;
    }
    return ledger;
}


entt::entity
cqsp::common::systems::actions::CreateCommercialArea(cqsp::common::Universe& universe, entt::entity city) {
    namespace cqspc = cqsp::common::components;
    entt::entity commercial = universe.create();

    universe.emplace<cqspc::Employer>(commercial);
    universe.emplace<cqspc::Commercial>(commercial, city, 0);

    universe.get<cqspc::IndustrialZone>(city).industries.push_back(commercial);
    return commercial;
}

