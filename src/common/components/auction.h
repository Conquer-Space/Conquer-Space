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

#include <functional>
#include <map>
#include <vector>

#include <entt/entt.hpp>

namespace cqsp::common::components {
struct Order {
    Order() : price(0), quantity(0) {}
    Order(double price, double quantity, entt::entity agent) : price(price), quantity(quantity), agent(agent) {}

    /// <summary>
    /// Price per unit
    /// </summary>
    double price;
    double quantity;

    /// <summary>
    /// The agent that owns this order. Goods sold will go to the agent, money recieved will
    /// also go to the agent. Needs a wallet component.
    /// </summary>
    entt::entity agent;
};

inline bool operator<(const Order& lhs, const Order& rhs) { return lhs.price < rhs.price; }

inline bool operator>(const Order& lhs, const Order& rhs) { return lhs.price > rhs.price; }

template <class T>
class SortedOrderList : public std::vector<Order> {
 public:
    using std::vector<Order>::vector;
    void put(const Order& order) {
        SortedOrderList<T>::iterator it = std::lower_bound(begin(), end(), order, T());
        insert(it, order);
    }
};

/// <summary>
/// Greatest to smallest
/// </summary>
typedef SortedOrderList<std::greater<Order>> DescendingSortedOrderList;

/// <summary>
/// Smallest to greatest
/// </summary>
typedef SortedOrderList<std::less<Order>> AscendingSortedOrderList;

struct AuctionHouse {
    std::map<entt::entity, DescendingSortedOrderList> sell_orders;
    std::map<entt::entity, AscendingSortedOrderList> buy_orders;

    void AddSellOrder(entt::entity good, Order&& order) { sell_orders[good].put(order); }

    void AddBuyOrder(entt::entity good, Order&& order) { buy_orders[good].put(order); }

    double GetDemand(entt::entity good) {
        const AscendingSortedOrderList& buy_list = buy_orders[good];
        double demand = 0;
        for (const Order& order : buy_list) {
            demand += order.quantity;
        }
        return demand;
    }

    double GetSupply(entt::entity good) {
        const DescendingSortedOrderList& sell_list = sell_orders[good];
        double supply = 0;
        for (const Order& order : sell_list) {
            supply += order.quantity;
        }
        return supply;
    }
};
}  // namespace cqsp::common::components
