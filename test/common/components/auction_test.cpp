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
#include <gtest/gtest.h>

#include <algorithm>

#include <iostream>

#include "common/components/auction.h"

using cqsp::common::components::DescendingSortedOrderList;
using cqsp::common::components::AscendingSortedOrderList;
using cqsp::common::components::Order;

TEST(AuctionTest, DescendingSortedOrderListTest) {
    DescendingSortedOrderList sorted_list;
    // Add random elements, and sort
    // quantity should not matter
    sorted_list.put(Order(40, 5));
    sorted_list.put(Order(45, 5));
    sorted_list.put(Order(14, 5));
    sorted_list.put(Order(59, 5));
    sorted_list.put(Order(12, 5));
    sorted_list.put(Order(50, 5));
    sorted_list.put(Order(20, 5));
    sorted_list.put(Order(80, 5));
    sorted_list.put(Order(10, 5));
    sorted_list.put(Order(157, 5));
    sorted_list.put(Order(45, 5));

    double previous = sorted_list[0].price;
    for (auto &i : sorted_list) {
        ASSERT_LE(i.price, previous);
        previous = i.price;
    }
}

TEST(AuctionTest, AscendingSortedOrderListTest) {
    AscendingSortedOrderList sorted_list;
    // Add random elements, and sort
    // quantity should not matter
    sorted_list.put(Order(40, 5));
    sorted_list.put(Order(45, 5));
    sorted_list.put(Order(14, 5));
    sorted_list.put(Order(59, 5));
    sorted_list.put(Order(12, 5));
    sorted_list.put(Order(50, 5));
    sorted_list.put(Order(20, 5));
    sorted_list.put(Order(80, 5));
    sorted_list.put(Order(10, 5));
    sorted_list.put(Order(157, 5));
    sorted_list.put(Order(45, 5));

    double previous = sorted_list[0].price;
    for (auto &i : sorted_list) {
        ASSERT_GE(i.price, previous);
        previous = i.price;
    }
}