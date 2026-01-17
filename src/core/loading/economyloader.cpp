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
#include "economyloader.h"

#define SET_ECONOMY_CONFIG(config_class, hjson_name, config_type)                          \
    do {                                                                                   \
        universe.economy_config.config_class.config_type = conf[#config_type].to_double(); \
    } while (0)

namespace cqsp::core::loading {

static const char* default_config_string = R"(
{
    market_config: {
        base_price_deviation: 0.75
        shortage_level: 0.8
        default_market_access: 0.8
    }
    production_config: {
        profit_multiplier: 0.001
        max_factory_delta: 0.01
        factory_min_utilization: 0.05
    }
}
)";

void LoadEconomyConfig(Universe& universe, const Hjson::Value& hjson) {
    // Set default config
    Hjson::Value default_config = Hjson::Unmarshal(default_config_string);
    Hjson::Value conf = Hjson::Merge(default_config, hjson);
    const Hjson::Value& market_config = conf["market_config"];
    SET_ECONOMY_CONFIG(market_config, market_config, base_price_deviation);
    SET_ECONOMY_CONFIG(market_config, market_config, shortage_level);
    SET_ECONOMY_CONFIG(market_config, market_config, default_market_access);
    const Hjson::Value& prod_config = conf["production_config"];
    SET_ECONOMY_CONFIG(production_config, prod_config, profit_multiplier);
    SET_ECONOMY_CONFIG(production_config, prod_config, max_factory_delta);
    SET_ECONOMY_CONFIG(production_config, prod_config, factory_min_utilization);
}
}  // namespace cqsp::core::loading
