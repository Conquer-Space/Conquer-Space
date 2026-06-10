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
#include "core/scripting/marketfunctions.h"

#include "core/components/history.h"
#include "core/components/market.h"
#include "core/components/organizations.h"
#include "core/scripting/functionreg.h"

namespace cqsp::core::scripting {
namespace {
void LoadCountryEconomyFunctions(Universe& universe, sol::state_view& script_engine) {
    CREATE_NAMESPACE(core);
    // now get gdp of player?
    REGISTER_FUNCTION("get_gdp", [&](entt::entity entity) {
        auto& city_list = universe.get<components::CountryCityList>(entity);
        // Now sort through the city
        double gdp = 0;
        for (entt::entity market : city_list.province_list) {
            gdp += universe.get<components::Market>(market).GDP;
        }
        return gdp;
    });
}
}  // namespace

void LoadMarketFunctions(Universe& universe, sol::state_view& script_engine) {
    LoadCountryEconomyFunctions(universe, script_engine);
    CREATE_NAMESPACE(core);
    lua_namespace.new_enum("GoodEntity", "null", components::GoodEntity::null);
    REGISTER_FUNCTION("good_entity_to_entity", [&](components::GoodEntity entity) { return universe.GetGood(entity); });

    lua_namespace.new_usertype<components::Market>("Market", sol::constructors<components::Market(size_t)>(), "GDP",
                                                   SOL_PROPERTY(components::Market, double, GDP), "price",
                                                   SOL_PROPERTY(components::Market, components::ResourceLedger, price));
    REGISTER_FUNCTION("get_market", [&](entt::entity entity) { return universe.get<components::Market>(entity); });

    // std::vector<double> usertype for indexing from Lua
    script_engine.new_usertype<std::vector<double>>(
        "DoubleVector", sol::no_constructor, sol::meta_function::index,
        [](const std::vector<double>& v, int i) -> sol::optional<double> {
            if (i < 0 || i >= static_cast<int>(v.size())) return sol::nullopt;
            return v[i];
        },
        sol::meta_function::length, [](const std::vector<double>& v) { return v.size(); });

    // std::vector<std::vector<double>> usertype — indexed by good index, returns DoubleVector
    script_engine.new_usertype<std::vector<std::vector<double>>>(
        "DoubleVectorVector", sol::no_constructor, sol::meta_function::index,
        [](std::vector<std::vector<double>>& v, int i) -> sol::optional<std::vector<double>*> {
            if (i < 0 || i > static_cast<int>(v.size())) return sol::nullopt;
            return &v[i];
        },
        sol::meta_function::length, [](const std::vector<std::vector<double>>& v) { return v.size(); });

    script_engine.new_usertype<components::ResourceLedger>(
        "ResourceLedger", sol::no_constructor, sol::meta_function::index,
        [](components::ResourceLedger& self, components::GoodEntity good) { return self[good]; }, "get_sum",
        &components::ResourceLedger::GetSum, "average", &components::ResourceLedger::Average, "min",
        &components::ResourceLedger::Min, "max", &components::ResourceLedger::Max, "size",
        &components::ResourceLedger::size);

    script_engine.new_usertype<components::MarketHistory>(
        "MarketHistory", sol::no_constructor, "price_history",
        SOL_PROPERTY(components::MarketHistory, std::vector<std::vector<double>>, price_history), "sd_ratio",
        SOL_PROPERTY(components::MarketHistory, std::vector<std::vector<double>>, sd_ratio), "supply",
        SOL_PROPERTY(components::MarketHistory, std::vector<std::vector<double>>, supply), "demand",
        SOL_PROPERTY(components::MarketHistory, std::vector<std::vector<double>>, demand), "gdp",
        SOL_PROPERTY(components::MarketHistory, std::vector<double>, gdp));

    script_engine.new_usertype<components::PopulationHistory>(
        "PopulationHistory", sol::no_constructor, "population",
        SOL_PROPERTY(components::PopulationHistory, std::vector<double>, population), "sol",
        SOL_PROPERTY(components::PopulationHistory, std::vector<double>, sol), "employment_rate",
        SOL_PROPERTY(components::PopulationHistory, std::vector<double>, employment_rate), "employment",
        SOL_PROPERTY(components::PopulationHistory, std::vector<double>, employment));

    REGISTER_FUNCTION("get_market_history", [&](entt::entity entity) -> components::MarketHistory& {
        return universe.get<components::MarketHistory>(entity);
    });

    REGISTER_FUNCTION("get_population_history", [&]() -> components::PopulationHistory& {
        return universe.ctx().at<components::PopulationHistory>();
    });
}
}  // namespace cqsp::core::scripting
