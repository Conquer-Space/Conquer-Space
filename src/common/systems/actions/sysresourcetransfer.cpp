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
#include "common/systems/actions/sysresourcetransfer.h"

#include "common/components/resource.h"

bool cqsp::common::systems::resource::TransferResources(entt::registry& registry, entt::entity from, entt::entity to,
                                                        entt::entity good, double amount) {
    namespace cqspc = cqsp::common::components;
    // Get resource stockpile
    if (!(registry.all_of<cqspc::ResourceStockpile>(from) && registry.all_of<cqspc::ResourceStockpile>(to) &&
          registry.all_of<cqspc::Good>(good))) {
        return false;
    }

    // Get resource stockpile
    auto& from_stockpile = registry.get<cqspc::ResourceStockpile>(from);
    auto& to_stockpile = registry.get<cqspc::ResourceStockpile>(from);
    // Transfer resources
    if (from_stockpile.HasGood(good)) {
        // Then we can transfer
        if (from_stockpile[good] >= amount) {
            from_stockpile[good] -= amount;
            to_stockpile[good] += amount;
            return true;
        }
    }
    return false;
}
