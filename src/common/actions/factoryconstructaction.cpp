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
#include "common/actions/factoryconstructaction.h"

#include <spdlog/spdlog.h>

#include <limits>
#include <map>

#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/components/resource.h"
#include "common/systems/economy/markethelpers.h"

namespace cqsp::common::systems::actions {
entt::entity OrderConstructionFactory(Universe& universe, entt::entity city, entt::entity market,
                                      entt::entity recipe, int productivity, entt::entity builder) {
    entt::entity factory = CreateFactory(universe, city, recipe, productivity);
    if (factory == entt::null) {
        return entt::null;
    }
    economy::AddParticipant(universe, market, factory);
    auto cost = GetFactoryCost(universe, city, recipe, productivity);

    // Buy the goods on the market
    economy::PurchaseGood(universe, builder, cost);
    return factory;
}

entt::entity CreateFactory(Universe& universe, entt::entity city, entt::entity recipe, int productivity) {
    // Make the factory
    if (city == entt::null || recipe == entt::null) {
        SPDLOG_WARN("City or recipe is null");
        return entt::null;
    }
    if (!universe.valid(city) || !universe.valid(recipe)) {
        SPDLOG_WARN("City or recipe is invalid");
        return entt::null;
    }
    if (!universe.any_of<components::IndustrialZone>(city)) {
        SPDLOG_WARN("City {} has no industry", city);
        return entt::null;
    }

    if (!universe.any_of<components::Recipe>(recipe)) {
        SPDLOG_WARN("Recipe {} has no recipe", recipe);
        return entt::null;
    }

    entt::entity factory = universe.create();
    //auto& factory_converter = universe.emplace<components::ResourceConverter>(factory);
    auto& production = universe.emplace<components::Production>(factory);
    // Add recipes and stuff
    production.recipe = recipe;
    universe.get<components::IndustrialZone>(city).industries.push_back(factory);

    // Add capacity
    // Add producivity
    auto& prod = universe.emplace<components::IndustrySize>(factory);
    prod.size = productivity;
    prod.utilization = productivity;
    const auto& recipe_comp = universe.get<components::Recipe>(recipe);
    switch (recipe_comp.type) {
        case components::mine:
            universe.emplace<components::Mine>(factory);
            break;
        case components::service:
            universe.emplace<components::Service>(factory);
            break;
        default:
            universe.emplace<components::Factory>(factory);
    }

    auto& employer = universe.emplace<components::Employer>(factory);
    // Set the employment amount, next time we can add other services like HR, tech, etc.
    employer.population_fufilled = 0;
    employer.population_needed = recipe_comp.workers * productivity;
    employer.segment = entt::null;
    return factory;
}

components::ResourceLedger GetFactoryCost(Universe& universe, entt::entity city, entt::entity recipe, int productivity) 
{
    components::ResourceLedger ledger;
    // Get the recipe and things
    if (universe.any_of<components::RecipeCost>(recipe)) {
        auto& cost = universe.get<components::RecipeCost>(recipe);
        ledger.MultiplyAdd(cost.scaling, productivity);
        ledger += cost.fixed;
    }
    return ledger;
}

entt::entity CreateCommercialArea(Universe& universe, entt::entity city) {
    entt::entity commercial = universe.create();

    universe.emplace<components::Employer>(commercial);
    universe.emplace<components::Commercial>(commercial, city, 0);

    universe.get<components::IndustrialZone>(city).industries.push_back(commercial);
    return commercial;
}
}  // namespace cqsp::common::systems::actions