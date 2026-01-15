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

#include <numeric>
#include <ranges>

#include "core/components/history.h"
#include "core/components/market.h"
#include "core/components/name.h"

namespace cqsp::core::systems::history {
void SysMarketCsvHistory::Init() {}

void SysMarketCsvHistory::DoSystem() {
    // Now log every system
    auto view = GetUniverse().view<components::LogMarket, components::Market>();
    for (entt::entity entity : view) {
        // Let's copy the market
        auto& market = GetUniverse().get<components::Market>(entity);

        const std::string& id = GetUniverse().get<components::Identifier>(entity).identifier;
        // Now we should make
        const std::string name = id + "_market.csv";
        if (!output_stream.contains(entity)) {
            output_stream[entity] = std::ofstream {name};
            // Paste the first row
            WriteCsvHeader(entity);
        }
        auto& stream = output_stream[entity];
        stream << GetUniverse().GetDate() << "," << market.GDP << "," << market.deficit << "," << market.last_deficit
               << "," << market.trade_deficit << "," << market.last_trade_deficit << ",";
        // Now let's write the thing
        auto double_list =
            GetUniverse().GoodIterator() | std::views::transform([&market](const components::GoodEntity good) {
                return std::to_string(market.supply[good]) + "," + std::to_string(market.demand[good]) + "," +
                       std::to_string(market.sd_ratio[good]) + "," + std::to_string(market.volume[good]) + "," +
                       std::to_string(market.price[good]) + "," + std::to_string(market.chronic_shortages[good]) + "," +
                       std::to_string(market.trade[good]) + "," + std::to_string(market.resource_fulfilled[good]) +
                       "," + std::to_string(market.production[good]) + "," + std::to_string(market.consumption[good]) +
                       ",";
            });
        stream << std::accumulate(
            double_list.begin(), double_list.end(), std::string(),
            [](const std::string& ss, const std::string& s) { return ss.empty() ? s : ss + "," + s; });
        stream << "\n";
    }
}

void SysMarketCsvHistory::WriteCsvHeader(const entt::entity entity) {
    auto& stream = output_stream[entity];
    stream << "Date,GDP,Deficit,Last Deficit,Trade Deficit,Last Trade Deficit,";

    auto row_list =
        (GetUniverse().GoodIterator() | std::views::transform([&](components::GoodEntity good) {
             const std::string& name =
                 GetUniverse().get<components::Identifier>(GetUniverse().GetGood(good)).identifier;
             std::vector<std::string> col_names = {"supply",     "demand",
                                                   "sd_ratio",   "volume",
                                                   "price",      "chronic_shortages",
                                                   "trade",      "resource_fufilled",
                                                   "production", "consumption"};

             auto view = col_names | std::ranges::views::transform(
                                         [name](const std::string& str) -> std::string { return name + "_" + str; });
             // When we upgrade to cpp23/cpp26 we can use the fold_left function to implement this
             // Literally Ocaml moment
             const std::string output_string = std::accumulate(
                 view.begin(), view.end(), std::string(),
                 [](const std::string& ss, const std::string& s) { return ss.empty() ? s : ss + "," + s; });

             return output_string;
         }));

    stream << std::accumulate(
        row_list.begin(), row_list.end(), std::string(),
        [](const std::string& ss, const std::string& s) { return ss.empty() ? s : ss + "," + s; });
    stream << "\n";
}
}  // namespace cqsp::core::systems::history
