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
#include "core/actions/factoryconstructaction.h"

#include <spdlog/spdlog.h>

#include <limits>
#include <map>

#include "core/actions/economy/markethelpers.h"
#include "core/components/area.h"
#include "core/components/market.h"
#include "core/components/resource.h"

namespace cqsp::core::actions {
Node OrderConstructionFactory(Node& city, Node& market, Node& recipe, Node& builder, int productivity) {
    Node factory = CreateFactory(city, recipe, productivity);
    if (factory.entity() == entt::null) {
        return Node(city.universe(), entt::null);
    }
    AddParticipant(market, factory);
    auto cost = GetFactoryCost(city, recipe, productivity);

    // Buy the goods on the market
    PurchaseGood(builder, cost);
    return factory;
}

Node CreateFactory(Node& city, Node& recipe, int productivity, double wages) {
    // Make the factory
    if (city == entt::null || recipe == entt::null) {
        SPDLOG_WARN("City or recipe is null");
        return Node(city.universe(), entt::null);
    }
    if (!city.valid() || !recipe.valid()) {
        SPDLOG_WARN("City or recipe is invalid");
        return Node(city.universe(), entt::null);
    }
    if (!city.any_of<components::IndustrialZone>()) {
        SPDLOG_WARN("City {} has no industry", city.entity());
        return Node(city.universe(), entt::null);
    }

    if (!recipe.any_of<components::Recipe>()) {
        SPDLOG_WARN("Recipe {} has no recipe", recipe.entity());
        return Node(city.universe(), entt::null);
    }

    Node factory(city.universe());
    //auto& factory_converter = universe.emplace<components::ResourceConverter>(factory);
    auto& production = factory.emplace<components::Production>();
    // Add recipes and stuff
    production.recipe = recipe;
    city.get_or_emplace<components::IndustrialZone>().industries.push_back(factory);

    // Add capacity
    // Add producivity
    auto& prod = factory.emplace<components::IndustrySize>();
    prod.size = productivity;
    prod.utilization = productivity;
    prod.wages = wages;
    const auto& recipe_comp = recipe.get<components::Recipe>();
    switch (recipe_comp.type) {
        case components::mine:
            factory.emplace<components::Mine>();
            break;
        case components::service:
            factory.emplace<components::Service>();
            break;
        default:
            factory.emplace<components::Factory>();
    }

    auto& employer = factory.emplace<components::Employer>();
    // Set the employment amount, next time we can add other services like HR, tech, etc.
    employer.population_fufilled = 0;
    employer.population_needed = recipe_comp.workers * productivity;
    employer.segment = entt::null;
    return factory;
}

components::ResourceLedger GetFactoryCost(Node& city, Node& recipe, int productivity) {
    components::ResourceLedger ledger;
    // Get the recipe and things
    if (recipe.any_of<components::RecipeCost>()) {
        auto& cost = recipe.get<components::RecipeCost>();
        ledger.MultiplyAdd(cost.scaling, productivity);
        ledger += cost.fixed;
    }
    return ledger;
}

Node CreateCommercialArea(Node& city) {
    Node commercial(city.universe());

    commercial.emplace<components::Employer>();
    commercial.emplace<components::Commercial>(city, 0);

    city.get<components::IndustrialZone>().industries.push_back(commercial);
    return commercial;
}
}  // namespace cqsp::core::actions