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
#include "core/systems/history/sysmarketcsvlogger.h"

#include "core/components/history.h"
#include "core/components/market.h"
#include "core/components/name.h"

namespace cqsp::core::systems::history {
void SysMarketCsvHistory::Init() {}

void SysMarketCsvHistory::DoSystem() {
    // Now log every system
    // auto view = GetUniverse().view<components::LogMarket, components::Market>();
    // for (entt::entity entity : view) {
    //     // Let's copy the market
    //     auto& market = GetUniverse().get<components::Market>(entity);

    //     const std::string& id = GetUniverse().get<components::Identifier>(entity).identifier;
    //     // Now we should make
    //     const std::string name = id + "_market.csv";
    //     if (!output_stream.contains(name)) {
    //         auto& stream = output_stream[entity] = std::ofstream{name};
    //         // Paste the first row
    //         stream << "Date,GDP,Deficit,Last Deficit,Trade Deficit,Last Deficit,";

    //         GetUniverse().GoodIterator() | std::ranges::view::transform([&](components::GoodEntity good) {
    //             const std::string& name = GetUniverse().get<components::Identifier>(GetUniverse().GetGood(good)).identifier;
    //             market.demand;
    //             market.supply;
    //         });;
    //         for(auto good : GetUniverse().GoodIterator()) {
    //             // Now paste the stuff
    //             const std::string& name = GetUniverse().get<components::Identifier>(GetUniverse().GetGood(good)).identifier;
    //             // Now make the different rows
    //         }
    //     }
    //     auto& stream = output_stream[entity];

    // }
}
}  // namespace cqsp::core::systems::history
