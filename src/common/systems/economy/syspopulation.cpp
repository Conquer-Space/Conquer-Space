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
#include "common/systems/economy/syspopulation.h"

#include <spdlog/spdlog.h>

#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/economy.h"

void cqsp::common::systems::SysPopulationGrowth::DoSystem() {
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    auto view = universe.view<cqspc::PopulationSegment>();
    for (auto [entity, segment] : view.each()) {
        // If it's hungry, decay population
        if (universe.all_of<cqspc::Hunger>(entity)) {
            // Population decrease will be about 1 percent each year.
            float increase = 1.f - static_cast<float>(Interval()) * 0.00000114077116f;
            segment.population *= increase;
        }

        if (universe.all_of<cqspc::FailedResourceTransfer>(entity)) {
            // Then alert hunger.
            universe.get_or_emplace<cqspc::Hunger>(entity);
        } else {
            universe.remove_if_exists<cqspc::Hunger>(entity);
        }
        // If not hungry, grow population
        if (!universe.all_of<cqspc::Hunger>(entity)) {
            // Population growth will be about 1 percent each year.
            float increase = static_cast<float>(Interval()) * 0.00000114077116f + 1;
            segment.population *= increase;
        }

        // Resolve jobs
        // TODO(EhWhoAmI)
        // For now, we would have 100% of the population working, because we haven't got to social simulation
        // yet. But in the future, this will probably have to change.
        auto& employee = universe.get_or_emplace<cqspc::Employee>(entity);
        employee.working_population = segment.population;
    }
}

void cqsp::common::systems::SysPopulationConsumption::DoSystem() {
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();

    const entt::entity good = universe.goods["consumer_good"];
    const entt::entity food = universe.goods["food"];
    auto view = universe.view<cqspc::PopulationSegment>();
    for (auto [entity, segment] : view.each()) {
        // The population will get at least an amount of resources, and
        // Reduce it to some unreasonably low level so that the economy can handle it
        uint64_t consumption = segment.population/100000;
        // Get the cost of food in the market, and then get the amount of food based
        // Essentially population units have a certain amount of food that they need,
        // and that's based on the population count.

        // They will buy all the food from the market, and the remaining will be
        // spent on consumer goods.

        // Other spendings we may want to think about:
        //  - Housing
        //  - Transport
        //  - Healthcare
        //  - Insurance
        //  - Entertainment
        //  - Education
        //  - Utilities
        auto& wallet = universe.get_or_emplace<cqspc::Wallet>(entity);
        // In the future, we may want to think about population wanting to save cash,
        // so to simulate consumer spending

        // Get the cost of the goods
        // Searching market is gonna be expensive, so we may need to reorganize the market
        // to maintain the cost
        if (universe.any_of<cqspc::MarketAgent>(entity)) {
            // Trade the goods
            auto& market_agent = universe.get<cqspc::MarketAgent>(entity);
            auto& market = universe.get<cqspc::Market>(market_agent.market);
            // Check if they have enough money to buy all they can, then
            double food_price = market.GetPrice(food);
            if (consumption * food_price > wallet.GetBalance()) {
                // Then not enough cash and pivot everything to buying food
                universe.get_or_emplace<cqspc::ResourceConsumption>(entity)[food] = wallet.GetBalance() / food_price;
            } else {
                universe.get_or_emplace<cqspc::ResourceConsumption>(entity)[food] = consumption;
                // Then waste it all on consumer goods
                double amount_consumer = wallet.GetBalance() - food_price * consumption;
                // If the price is too high, then don't buy, it's not worth it, we'll wait for the price to crash
                double good_price = market.GetPrice(good);
                if (amount_consumer / good_price > segment.population / 20000) {
                    universe.get_or_emplace<cqspc::ResourceConsumption>(entity)[good] =
                                                        amount_consumer / good_price;
                }
                // Based on how much they spend on consumer goods, we can rate their social strata
                // becasue those that consume more will have a higher standard of living.
            }
        } else {
            universe.get_or_emplace<cqspc::ResourceConsumption>(entity)[food] = consumption;
        }

        // Inject some cash into the population segment, so that they don't run out of money to buy the stuff
        wallet += segment.population / 1000;
    }
}
