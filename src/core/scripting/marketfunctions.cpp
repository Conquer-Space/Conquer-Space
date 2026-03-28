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
#include "core/scripting/functionreg.h"

// Helper function to get around sol's error
// NOLINTBEGIN: We are supposed to add parentheses around the macro parameters but it's not possible because
// the macro won't work
#define SOL_PROPERTY(type, prop_type, name) \
    sol::property(([](type& self) { return self.name; }), ([](type& self, prop_type name) { self.name = name; }))
// NOLINTEND
namespace cqsp::core::scripting {
void LoadMarketFunctions(Universe& universe, sol::state_view& script_engine) {
    CREATE_NAMESPACE(core);

    // Register orbit data
    lua_namespace.new_usertype<components::Market>("Market", sol::constructors<components::Market(size_t)>(), "GDP",
                                                   SOL_PROPERTY(components::Market, double, GDP));
    REGISTER_FUNCTION("get_market", [&](entt::entity entity) { return universe.get<components::Market>(entity); });

    // std::vector<double> usertype for indexing from Lua (1-based)
    script_engine.new_usertype<std::vector<double>>(
        "DoubleVector", sol::no_constructor, sol::meta_function::index,
        [](const std::vector<double>& v, int i) -> sol::optional<double> {
            if (i < 1 || i > static_cast<int>(v.size())) return sol::nullopt;
            return v[i - 1];
        },
        sol::meta_function::length, [](const std::vector<double>& v) { return v.size(); });

    // std::vector<std::vector<double>> usertype — indexed by good index (1-based), returns DoubleVector
    script_engine.new_usertype<std::vector<std::vector<double>>>(
        "DoubleVectorVector", sol::no_constructor, sol::meta_function::index,
        [](std::vector<std::vector<double>>& v, int i) -> sol::optional<std::vector<double>*> {
            if (i < 1 || i > static_cast<int>(v.size())) return sol::nullopt;
            return &v[i - 1];
        },
        sol::meta_function::length, [](const std::vector<std::vector<double>>& v) { return v.size(); });

    script_engine.new_usertype<components::MarketHistory>(
        "MarketHistory", sol::no_constructor, "price_history", &components::MarketHistory::price_history, "sd_ratio",
        &components::MarketHistory::sd_ratio, "supply", &components::MarketHistory::supply, "demand",
        &components::MarketHistory::demand, "gdp", &components::MarketHistory::gdp);

    REGISTER_FUNCTION("get_market_history", [&](entt::entity entity) -> components::MarketHistory& {
        return universe.get<components::MarketHistory>(entity);
    });
}
}  // namespace cqsp::core::scripting
