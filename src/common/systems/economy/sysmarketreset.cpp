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
#include "common/systems/economy/sysmarketreset.h"

#include "common/components/economy.h"

namespace cqsp::common::systems {
void SysMarketReset::DoSystem() {
    auto marketview = GetUniverse().view<components::Market>();
    Universe& universe = GetUniverse();
    for (entt::entity entity : marketview) {
        // Reset the ledgers
        components::Market& market = universe.get<components::Market>(entity);
        market.ResetLedgers();
        market.imports.clear();
        market.exports.clear();
        market.production.clear();
        market.consumption.clear();
        market.latent_supply.clear();
        market.latent_demand.clear();
    }
}
}  // namespace cqsp::common::systems
