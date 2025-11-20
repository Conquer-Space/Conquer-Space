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
#include "core/components/auction.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>

#include "core/actions/economy/auctionhandler.h"

using cqsp::core::components::AscendingSortedOrderList;
using cqsp::core::components::AuctionHouse;
using cqsp::core::components::DescendingSortedOrderList;
using cqsp::core::components::Order;

cqsp::core::Universe universe;
cqsp::core::Node test_good(universe);
cqsp::core::Node test_agent(universe);

TEST(AuctionTest, DescendingSortedOrderListTest) {
    DescendingSortedOrderList sorted_list;
    // Add random elements, and sort
    // quantity should not matter
    sorted_list.put(Order(40, 5, test_agent));
    sorted_list.put(Order(45, 5, test_agent));
    sorted_list.put(Order(14, 5, test_agent));
    sorted_list.put(Order(59, 5, test_agent));
    sorted_list.put(Order(12, 5, test_agent));
    sorted_list.put(Order(50, 5, test_agent));
    sorted_list.put(Order(20, 5, test_agent));
    sorted_list.put(Order(80, 5, test_agent));
    sorted_list.put(Order(10, 5, test_agent));
    sorted_list.put(Order(157, 5, test_agent));
    sorted_list.put(Order(45, 5, test_agent));

    double previous = sorted_list[0].price;
    for (auto &i : sorted_list) {
        EXPECT_LE(i.price, previous);
        previous = i.price;
    }
}

TEST(AuctionTest, AscendingSortedOrderListTest) {
    AscendingSortedOrderList sorted_list;
    // Add random elements, and sort
    // quantity should not matter
    sorted_list.put(Order(40, 5, test_agent));
    sorted_list.put(Order(45, 5, test_agent));
    sorted_list.put(Order(14, 5, test_agent));
    sorted_list.put(Order(59, 5, test_agent));
    sorted_list.put(Order(12, 5, test_agent));
    sorted_list.put(Order(50, 5, test_agent));
    sorted_list.put(Order(20, 5, test_agent));
    sorted_list.put(Order(80, 5, test_agent));
    sorted_list.put(Order(10, 5, test_agent));
    sorted_list.put(Order(157, 5, test_agent));
    sorted_list.put(Order(45, 5, test_agent));

    double previous = sorted_list[0].price;
    for (auto &i : sorted_list) {
        EXPECT_GE(i.price, previous);
        previous = i.price;
    }
}

TEST(AuctionTest, DemandTest) {
    AuctionHouse auction_house;
    // Price is irrelevant
    static const int size = 10;
    double test_orders[size] = {3, 56, 15, 75332, 784, 329, 43, 786, 2536, 19};
    double total_demand = 0;
    for (int i = 0; i < size; i++) {
        auction_house.AddBuyOrder(test_good, Order(10, test_orders[i], test_agent));
        total_demand += test_orders[i];
    }
    EXPECT_EQ(total_demand, auction_house.GetDemand(test_good));
}

TEST(AuctionTest, SupplyTest) {
    AuctionHouse auction_house;
    // Price is irrelevant
    static const int size = 10;
    double test_orders[size] = {3, 56, 15, 75332, 784, 329, 43, 786, 2536, 19};
    double total_demand = 0;
    for (int i = 0; i < size; i++) {
        auction_house.AddSellOrder(test_good, Order(10, test_orders[i], test_agent));
        total_demand += test_orders[i];
    }
    EXPECT_EQ(total_demand, auction_house.GetSupply(test_good));
}

TEST(AuctionTest, BasicBuyOrderTest) {
    AuctionHouse auction_house;
    // Add basic buy order
    auction_house.AddSellOrder(test_good, Order(10, 50, test_agent));

    EXPECT_EQ(auction_house.sell_orders[test_good].size(), 1);
    EXPECT_EQ(static_cast<int>(auction_house.GetSupply(test_good)), 50);
    bool is_ordered = cqsp::core::actions::BuyGood(auction_house, test_agent, test_good, 10, 50);

    // Ensure it's fufilled immediately
    EXPECT_TRUE(is_ordered);

    // Ensure the buy ordered is fufilled
    EXPECT_TRUE(auction_house.buy_orders[test_good].empty());
    EXPECT_TRUE(auction_house.sell_orders[test_good].empty());
}

// Test for buy orders that cannot be fully fufilled due to quantity
TEST(AuctionTest, UnfufilledBuyOrderTest) {
    AuctionHouse auction_house;
    // Add basic buy order
    auction_house.AddSellOrder(test_good, Order(10, 100, test_agent));
    bool is_ordered = cqsp::core::actions::BuyGood(auction_house, test_agent, test_good, 10, 50);

    // It's fufilled immediately
    EXPECT_TRUE(is_ordered);

    // ensure that sell order is not totally fufilled
    EXPECT_FALSE(auction_house.sell_orders[test_good].empty());

    // ensure no buy orders are sold
    EXPECT_TRUE(auction_house.buy_orders[test_good].empty());

    // They should have 50 test goods left
    EXPECT_EQ(50, auction_house.GetSupply(test_good));
}

// Demand is way higher
TEST(AuctionTest, OverfufilledBuyOrderTest) {
    AuctionHouse auction_house;
    auction_house.AddSellOrder(test_good, Order(10, 100, test_agent));

    bool is_ordered = cqsp::core::actions::BuyGood(auction_house, test_agent, test_good, 10, 1000);

    EXPECT_FALSE(is_ordered);

    // Sell order is fufilled
    EXPECT_TRUE(auction_house.sell_orders[test_good].empty());
    EXPECT_FALSE(auction_house.buy_orders[test_good].empty());

    EXPECT_EQ(900, auction_house.GetDemand(test_good));
}

// Good is overpriced, so they don't want to buy
TEST(AuctionTest, OverpricedBuyOrderTest) {
    AuctionHouse auction_house;
    // Add basic buy order
    auction_house.AddSellOrder(test_good, Order(1000, 100, test_agent));
    bool is_ordered = cqsp::core::actions::BuyGood(auction_house, test_agent, test_good, 10, 100);

    // It's fufilled immediately
    EXPECT_FALSE(is_ordered);

    // Ensure that sell order is not totally fufilled
    EXPECT_FALSE(auction_house.sell_orders[test_good].empty());

    // Ensure there's a buy order
    EXPECT_FALSE(auction_house.buy_orders[test_good].empty());

    // They should have 50 test goods left
    EXPECT_EQ(100, auction_house.GetSupply(test_good));
    EXPECT_EQ(100, auction_house.GetDemand(test_good));
}

// Sell orders are basically the same as buy orders, except they are now sell orders.
TEST(AuctionTest, BasicSellOrderTest) {
    AuctionHouse auction_house;
    // Add basic buy order
    auction_house.AddBuyOrder(test_good, Order(10, 50, test_agent));
    bool is_ordered = cqsp::core::actions::SellGood(auction_house, test_agent, test_good, 10, 50);

    // Ensure it's fufilled immediately
    EXPECT_TRUE(is_ordered);

    // Ensure the buy ordered is fufilled, and no sell order is added
    EXPECT_TRUE(auction_house.buy_orders[test_good].empty());
    EXPECT_TRUE(auction_house.sell_orders[test_good].empty());
}

// Test for buy orders that cannot be fully fufilled due to quantity
TEST(AuctionTest, UnfufilledSellOrderTest) {
    AuctionHouse auction_house;
    // Add basic buy order
    auction_house.AddBuyOrder(test_good, Order(10, 100, test_agent));
    bool is_ordered = cqsp::core::actions::SellGood(auction_house, test_agent, test_good, 10, 50);

    // It's fufilled immediately
    EXPECT_TRUE(is_ordered);

    // ensure that buy order is not totally fufilled
    EXPECT_FALSE(auction_house.buy_orders[test_good].empty());

    // ensure no sell orders are added
    EXPECT_TRUE(auction_house.sell_orders[test_good].empty());

    // They should have 50 test goods left
    EXPECT_EQ(50, auction_house.GetDemand(test_good));
}

// Supply is way higher
TEST(AuctionTest, OverfufilledSellOrderTest) {
    AuctionHouse auction_house;
    auction_house.AddBuyOrder(test_good, Order(10, 100, test_agent));

    bool is_ordered = cqsp::core::actions::SellGood(auction_house, test_agent, test_good, 10, 1000);

    EXPECT_FALSE(is_ordered);

    // Sell order is fufilled
    EXPECT_FALSE(auction_house.sell_orders[test_good].empty());
    EXPECT_TRUE(auction_house.buy_orders[test_good].empty());

    EXPECT_EQ(900, auction_house.GetSupply(test_good));
    EXPECT_EQ(0, auction_house.GetDemand(test_good));
}

// Good is uunder, so they don't want to sell
TEST(AuctionTest, OverpricedSellOrderTest) {
    AuctionHouse auction_house;
    // Add basic buy order
    auction_house.AddBuyOrder(test_good, Order(10, 100, test_agent));
    bool is_ordered = cqsp::core::actions::SellGood(auction_house, test_agent, test_good, 1000, 100);

    // It's not fufilled immediately
    EXPECT_FALSE(is_ordered);

    // Ensure that sell order is not totally fufilled
    EXPECT_FALSE(auction_house.sell_orders[test_good].empty());

    // Ensure there's a buy order
    EXPECT_FALSE(auction_house.buy_orders[test_good].empty());

    EXPECT_EQ(100, auction_house.GetDemand(test_good));
    EXPECT_EQ(100, auction_house.GetSupply(test_good));
}
