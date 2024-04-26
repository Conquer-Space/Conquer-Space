/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

namespace components = cqsp::common::components;
using components::ResourceStockpile;
using components::Good;
using entt::entity;
namespace cqsp::common::systems::resource
{
    bool TransferResources(entt::registry& registry, entity from, entity to, entity good, double amount) {
        // Get resource stockpile
        if (!(registry.all_of<ResourceStockpile>(from) && registry.all_of<ResourceStockpile>(to) &&
              registry.all_of<Good>(good))) {
            return false;
        }

        // Get resource stockpile
        auto& from_stockpile = registry.get<ResourceStockpile>(from);
        auto& to_stockpile = registry.get<ResourceStockpile>(from);
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
}

