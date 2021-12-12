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
#include "common/systems/economy/auctionhandler.h"

#include <spdlog/spdlog.h>
bool cqsp::common::systems::BuyGood(components::AuctionHouse& auction_house, entt::entity good, double price, double quantity) {
    // The orders we want to try and fufill
    auto& sell_order_list = auction_house.sell_orders[good];
    auto& buy_order_list = auction_house.buy_orders[good];

    // Check if goods are being sold
    if (sell_order_list.empty()) {
        // Then make new buy order
        components::Order order(price, quantity);
        buy_order_list.put(order);
        return false;
    }
    // Then pop the first in the list, and check if the price works
    while (!sell_order_list.empty()) {
        components::Order& first = sell_order_list.front();
        // Then check if the price is right
        if (first.price > price) {
            break;
        }
        // Buy good, and fufill demand
        if (first.quantity > quantity) {
            first.quantity -= quantity;
            quantity = 0;
            // Also subtract wallet
            // Thing done!
            return true;
        } else {
            // remove order, remove quantity
            quantity -= first.quantity;
            // Remove buy order
            sell_order_list.erase(sell_order_list.begin());
        }
    }

    if (quantity <= 0) {
        return true;
    }
    // Then place a buy order because the order could not be fufulled.
    components::Order order(price, quantity);
    buy_order_list.put(order);
    return false;
}

bool cqsp::common::systems::SellGood(components::AuctionHouse& auction_house,
                                     entt::entity good, double price,
                                     double quantity) {
    // The place we will put our order if we cannot fufill it
    auto& sell_order_list = auction_house.sell_orders[good];

    // The orders we want to try and fufill
    auto& buy_order_list = auction_house.buy_orders[good];

    // Check if goods are being sold
    if (buy_order_list.empty()) {
        // Then make new order
        components::Order order(price, quantity);
        sell_order_list.put(order);
        return false;
    }
    // Then pop the first in the list, and check if the price works
    while (!buy_order_list.empty()) {
        components::Order& first = buy_order_list.front();
        // Then check if the price is right
        if (first.price < price) {
            break;
        }
        // Buy good, and fufill demand
        if (first.quantity > quantity) {
            first.quantity -= quantity;
            quantity = 0;
            // Also subtract wallet
            // Thing done!
            return true;
        } else {
            // remove order, remove quantity
            quantity -= first.quantity;
            // Remove buy order
            buy_order_list.erase(buy_order_list.begin());
            if (quantity == 0) {
                break;
            }
        }
    }

    if (quantity <= 0) {
        return true;
    }
    // Then place a sell order because the order could not be fufulled.
    components::Order order(price, quantity);
    sell_order_list.put(order);
    return false;
}
