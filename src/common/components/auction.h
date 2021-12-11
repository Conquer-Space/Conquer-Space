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

#include <vector>

#include <entt/entt.hpp>

namespace cqsp {
namespace common {
namespace components {
struct Order {
    Order() : price(0), quantity(0) { }
    Order(double price, double quantity) : price(price), quantity(quantity) { }

    /// <summary>
    /// Price per unit
    /// </summary>
    double price;
    double quantity;
};

bool operator<(const Order& lhs, const Order& rhs) {
    return lhs.price < rhs.price;
}

bool operator>(const Order& lhs, const Order& rhs) {
    return lhs.price > rhs.price;
}

template<class T>
class SortedOrderList : public std::vector<Order> {
 public:
     using std::vector<Order>::vector;
     void put(Order &&order) {
         DescendingSortedOrderList::iterator it =
             std::lower_bound(begin(), end(), order, T());
         insert(it, order); // insert before iterator it
     }};
/// <summary>
/// Greatest to smallest
/// </summary>
typedef SortedOrderList<std::greater<Order>> DescendingSortedOrderList;

/// <summary>
/// Smallest to greatest
/// </summary>
typedef SortedOrderList<std::less<Order>> AscendingSortedOrderList;

class AuctionHouse {
    std::map<entt::entity, DescendingSortedOrderList> sell_orders;
    std::map<entt::entity, AscendingSortedOrderList> buy_orders;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
