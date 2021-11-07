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
#include "common/systems/economy/sysfactory.h"

#include <spdlog/spdlog.h>

#include "common/components/area.h"
#include "common/components/resource.h"
#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/infrastructure.h"

void cqsp::common::systems::SysFactory::DoSystem(Universe& universe) {
    SysInfrastrutureChecker(universe);
    SysFactoryProduction(universe);
    SysMineProduction(universe);
}

void cqsp::common::systems::SysFactory::SysFactoryProduction(Universe& universe) {
    // When demand is higher, increase production, when demand is lower, decrease production.
    namespace cqspc = cqsp::common::components;

    auto view = universe.view<cqspc::ResourceConverter, cqspc::FactoryProductivity, cqspc::MarketAgent>();
    for (entt::entity entity : view) {
        // Get market and the supply, and determine if you have to generate more or less, based
        // upon maximum production
        auto& agent = universe.get<cqspc::MarketAgent>(entity);
        auto& market = universe.get<cqspc::Market>(agent.market);
        auto& factory = universe.get<cqspc::ResourceConverter>(entity);
        auto& output = universe.get<cqspc::Recipe>(factory.recipe).output;
        for (auto it = output.begin(); it != output.end(); it++) {
            // Get supply and demand of the good
            if (market.sd_ratio[it->first] > 1) {
                // Decrease production due to low demand
                // Reduce by 10%.
                // TODO(EhWhoAmI): Tweak this so that this would take into account competitors,
                // and also take into account how large the S/D ratio is, so that they can
                // drastically or minimally change the price of the good as needed.
                universe.get<cqspc::FactoryProductivity>(entity).productivity *= 0.9;
            } else {
                // Then increase production due to the high demand
                float& prod = universe.get<cqspc::FactoryProductivity>(entity).productivity;
                // If productivity is close to zero, then multiply it by a factor of it's maximum capicity
                if (prod >= -0.01 && prod <= 0.01) {
                    prod = universe.get<cqspc::FactoryCapacity>(entity).capacity * 0.1;
                }
                prod *= 1.1;
            }
        }
    }
}

void cqsp::common::systems::SysFactory::SysMineProduction(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    auto view = universe.view<cqspc::Mine, cqspc::ResourceGenerator, cqspc::MarketAgent, cqspc::FactoryProductivity>();
    for (entt::entity entity : view) {
        // Get market and the supply, and determine if you have to generate more or less, based
        // upon maximum production
        auto& agent = universe.get<cqspc::MarketAgent>(entity);
        auto& market = universe.get<cqspc::Market>(agent.market);
        auto& factory = universe.get<cqspc::ResourceGenerator>(entity);
        for (auto it = factory.begin(); it != factory.end(); it++) {
            // Get supply and demand of the good
            double sd_ratio = market.sd_ratio[it->first];
            if (sd_ratio > 1) {
                // Decrease production due to low demand
                // Reduce by 10%.
                // TODO(EhWhoAmI): Tweak this so that this would take into account competitors,
                // and also take into account how large the S/D ratio is, so that they can
                // drastically or minimally change the price of the good as needed.
                universe.get<cqspc::FactoryProductivity>(entity).productivity *= 0.9;
            } else if (sd_ratio < 1) {
                // Then increase production due to the high demand
                float& prod = universe.get<cqspc::FactoryProductivity>(entity).productivity;
                // If productivity is close to zero, then multiply it by a factor of it's maximum capicity
                if (prod >= -0.01 && prod <= 0.01) {
                    prod = universe.get<cqspc::FactoryCapacity>(entity).capacity * 0.1;
                }
                prod *= 1.1;
            }
        }
    }
}

void cqsp::common::systems::SysFactory::SysInfrastrutureChecker(Universe& universe) {
    namespace cqspc = cqsp::common::components;
    // Checks cities for power and other things
    auto view = universe.view<cqspc::infrastructure::CityPower, cqspc::Industry>();
    for (entt::entity entity : view) {
        auto& industry_comp = universe.get<cqspc::Industry>(entity);
        auto& power_supply = universe.get<cqspc::infrastructure::CityPower>(entity);
        for(entt::entity industry : industry_comp.industries) {
            if (universe.any_of<cqspc::infrastructure::PowerConsumption>(industry)) {
                // Then set the power consumption
                auto& power = universe.get<cqspc::infrastructure::PowerConsumption>(industry);
                if (power_supply.total_power_consumption > power_supply.total_power_prod) {
                    power.current = power_supply.total_power_prod /
                                power_supply.total_power_consumption *
                                power.max;
                }
                power.current = power.max;
            }
        }
    }
}

void cqsp::common::systems::SysFactory::SysPowerProcessor(Universe& universe) {
    namespace cqspc = cqsp::common::components;

    auto view = universe.view<cqspc::FactoryProductivity, cqspc::infrastructure::PowerConsumption>();
    for (entt::entity entity : view) {
        //universe.get<cqspc::FactoryProductivity>();
        auto& power_consumption = universe.get<cqspc::infrastructure::PowerConsumption>(entity);
        double prod = power_consumption.current / power_consumption.min;
        // Modify production
        universe.get<cqspc::FactoryProductivity>(entity);
    }
}
