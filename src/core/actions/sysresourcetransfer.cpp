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
#include "core/actions/sysresourcetransfer.h"

#include "core/components/resource.h"

namespace cqsp::core::actions {
using components::ResourceStockpile;

bool TransferResources(Node& from, Node& to, Node& good, double amount) {
    // Get resource stockpile
    if (!(from.all_of<ResourceStockpile>() && to.all_of<ResourceStockpile>() && good.all_of<components::Good>())) {
        return false;
    }

    // Get resource stockpile
    auto& from_stockpile = from.get<ResourceStockpile>();
    auto& to_stockpile = from.get<ResourceStockpile>();
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
}  // namespace cqsp::core::actions