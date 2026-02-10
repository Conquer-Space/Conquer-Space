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
#include "core/systems/history/sysmarketdumper.h"

#include <hjson.h>

#include <fstream>

#include "core/components/area.h"
#include "core/components/name.h"

namespace cqsp::core::systems::history {
void SaveUniverseMarketState(Universe& universe, const std::string& output_file_name) {
    auto zone_view = universe.view<components::IndustrialZone>();
    // We should match the factories to the entity
    Hjson::Value compilation;
    for (entt::entity zone : zone_view) {
        auto industries = universe.get<components::IndustrialZone>(zone);
        Hjson::Value city_hjson;
        for (Node industry_node : universe.Convert(industries.industries)) {
            if (!industry_node.all_of<components::IndustrySize, components::CostBreakdown>()) {
                continue;
            }
            Hjson::Value industry_hjson;
            auto& size = industry_node.get<components::IndustrySize>();
            auto& costs = industry_node.get<components::CostBreakdown>();

            // Also get the identifier for the good
            auto& factory = industry_node.get<components::Production>();
            industry_hjson["recipe"] = universe.get<components::Identifier>(factory.recipe).identifier;

            industry_hjson["size"] = size.size;
            industry_hjson["utilization"] = size.utilization;
            industry_hjson["workers"] = size.workers;
            industry_hjson["wages"] = size.wages;
            industry_hjson["continuous_losses"] = size.continuous_losses;
            industry_hjson["continuous_gains"] = size.continuous_gains;

            industry_hjson["revenue"] = costs.revenue;
            industry_hjson["material_costs"] = costs.material_costs;
            industry_hjson["profit"] = costs.profit;
            // Now let's save everything
            city_hjson.push_back(industry_hjson);
        }
        compilation[universe.get<components::Identifier>(zone).identifier] = city_hjson;
    }
    std::ofstream output_file(output_file_name);
    Hjson::EncoderOptions enc_opt;
    enc_opt.indentBy = "    ";
    output_file << Hjson::StreamEncoder(compilation, enc_opt);
}
}  // namespace cqsp::core::systems::history
