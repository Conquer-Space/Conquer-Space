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
#pragma once

#include <entt/entt.hpp>

#include "core/components/auction.h"
#include "core/universe.h"

namespace cqsp::core::actions {
/// <summary>
/// Buys a good from the market
/// </summary>
/// <param name="auction_house">Auction house to buy from</param>
/// <param name="good">Good to buy</param>
/// <param name="price">Price</param>
/// <param name="quantity">Quantity</param>
/// <returns>True if the order is fufilled immediately, false if a buy order is
/// placed.</returns>
bool BuyGood(components::AuctionHouse& auction_house, Node& agent, Node& good, double price, double quantity);

/// <summary>
/// Sells a good to the market
/// </summary>
/// <param name="auction_house">Auction house to buy from</param>
/// <param name="good">Good to buy</param>
/// <param name="price">Price</param>
/// <param name="quantity">Quantity</param>
/// <returns>True if the order is fufilled immediately, false if a sell order is
/// placed.</returns>
bool SellGood(components::AuctionHouse& auction_house, Node& agent, Node&, double price, double quantity);
}  // namespace cqsp::core::actions
