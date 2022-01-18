/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#pragma once

#include <entt/entt.hpp>

#include "common/universe.h"
#include "common/components/resource.h"

namespace cqsp {
namespace common {
namespace systems {
namespace economy {
/// <summary>
/// Creates a market instance.
/// </summary>
[[nodiscard]]
entt::entity CreateMarket(Universe& universe);

/// <summary>
/// Makes the market entity a market instance.
/// </summary>
[[maybe_unused]]
void CreateMarket(Universe& universe, entt::entity market);
/// <summary>
/// Note: This will only buy the maximum resources that are in the market.
/// If there aren't enough resources on the market, then we buy all the
/// remaining resources on the market.
/// You'll have to calculate how much you want later on
/// </summary>
/// <param name="universe"></param>
/// <param name="agent"></param>
/// <param name="purchase"></param>
/// <returns></returns>
bool PurchaseGood(Universe& universe, entt::entity agent,
                  const components::ResourceLedger & purchase);
bool SellGood(Universe& universe, entt::entity agent,
              const components::ResourceLedger & selling);

void AddParticipant(cqsp::common::Universe& universe, entt::entity market, entt::entity entity);
}  // namespace economy
}  // namespace systems
}  // namespace common
}  // namespace cqsp
