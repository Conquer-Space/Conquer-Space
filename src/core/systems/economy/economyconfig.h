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
#pragma once

#include "core/components/stardate.h"

namespace cqsp::core::systems {
const int ECONOMIC_TICK = components::StarDate::HOUR;
/**
 * A struct for all the configs that we want to configure for the economy
 */
struct EconomyConfig {
    struct {
        /**
         * Fraction that the price of a good can deviate from the base price.
         */
        double base_price_deviation = 0.75;
        /**
         * How much of a deficit we have ((demand - supply) / demand)
         * to be considered a shortage.
         */
        double shortage_level = 0.8;

        /**
         * What to initialize our market access with.
         */
        double default_market_access = 0.8;
    } market_config;

    struct {
        /**
         * Multiplier for how much we should increase or decrease production based off profit
         */
        double profit_multiplier = 0.001;
        /**
         * The maximum percentage change per economic tick we should modify our economy.
         */
        double max_factory_delta = 0.01;
        /**
         * The percentage of a factory that has to be in use
         */
        double factory_min_utilization = 0.05;
    } production_config;
};
}  // namespace cqsp::core::systems
