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
#include "core/actions/economy/subsidyhelper.h"

#include "core/components/organizations.h"

namespace cqsp::core::actions {
void ApplySubsidy(Universe& universe, entt::entity country, entt::entity subsidy_target, float amount) {
    // Then get our country and apply our subsidy
    auto& country_list = universe.get<components::CountryCityList>(country);
    auto& subsidies = universe.get<components::Subsidies>(country);

    subsidies.global_subsidy[subsidy_target] = amount;
    // Now apply the subsidy
    for (entt::entity province : country_list.province_list) {
        auto& industries = universe.get<components::IndustrialZone>(province);
        for (Node industry_node : universe.Convert(industries.industries)) {
            // We should also check for industries we want to construct
            auto& industry = industry_node.get<components::ProductionUnit>();
            if (industry.recipe == subsidy_target) {
                industry.output_subsidy = amount;
            }
        }
    }
}
void RemoveSubsidy() {}
}  // namespace cqsp::core::actions
