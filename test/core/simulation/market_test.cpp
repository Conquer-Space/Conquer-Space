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
#include "core/components/market.h"

#include <gtest/gtest.h>

#include <iostream>

#include "core/actions/economy/markethelpers.h"
#include "core/systems/economy/sysmarket.h"
#include "core/universe.h"

namespace cqspc = cqsp::core::components;

class MarketTwoTest : public ::testing::Test {
 protected:
    cqsp::core::Game game;
    cqsp::core::Universe& universe;
    cqsp::core::Node agent1;
    cqsp::core::Node agent2;
    cqsp::core::Node market;
    cqsp::core::Node good_1;
    cqsp::core::Node good_2;
    const int good_1_default_price = 100;
    const int good_2_default_price = 300;

    MarketTwoTest()
        : universe(game.GetUniverse()),
          agent1(universe),
          agent2(universe),
          market(cqsp::core::actions::CreateMarket(universe)),
          good_1(universe),
          good_2(universe) {}

    void SetUp() override {
        auto& market_comp = market.get<cqspc::Market>();
        market_comp[good_1].price = good_1_default_price;
        market_comp[good_2].price = good_2_default_price;

        // Setup agents
        agent1.emplace<cqspc::Wallet>();
        agent1.emplace<cqspc::ResourceStockpile>();
        agent1.emplace<cqspc::MarketAgent>(market);

        agent2.emplace<cqspc::Wallet>();
        agent2.emplace<cqspc::ResourceStockpile>();
        agent2.emplace<cqspc::MarketAgent>(market);
    }
};

TEST_F(MarketTwoTest, SellTest) {
    // Then test if the prices fluctuate or something
    // Add resources
    auto& stockpile = agent1.get<cqspc::ResourceStockpile>();
    stockpile[good_1] = 100;
    // Now test sell the goods
    EXPECT_EQ(stockpile[good_1], 100);
    ASSERT_TRUE(cqsp::core::actions::SellGood(agent1, stockpile));
    // Then check if the goods are sold
    auto& market_comp = market.get<cqspc::Market>();

    // Check if goods are on the market
    EXPECT_EQ(market_comp[good_1].supply, 100);

    // Check if agent sold resources
    EXPECT_EQ(stockpile[good_1], 0);
    // So the supply is 100 now
    // Check if wallet is subtracted
    auto& wallet = agent1.get<cqspc::Wallet>();
    EXPECT_EQ(wallet.GetBalance(), good_1_default_price * 100);
}

TEST_F(MarketTwoTest, BuyTest) {
    // Add money to the wallet so that they have enough to buy
    agent1.get<cqspc::Wallet>() = 100 * good_1_default_price;

    // Now test sell the goods
    cqspc::ResourceStockpile to_buy;
    to_buy[good_1] = 100;
    ASSERT_TRUE(cqsp::core::actions::PurchaseGood(agent1, to_buy));

    auto& market_comp = market.get<cqspc::Market>();

    // Check demand
    EXPECT_EQ(market_comp[good_1].demand, 100);

    // Check wallet
    EXPECT_EQ(agent1.get<cqspc::Wallet>().GetBalance(), 0);
}

TEST_F(MarketTwoTest, BuySellTest) {
    // Add the resources and sell them, then try buying them, then get the change in price
    agent1.get<cqspc::Wallet>() = 100 * good_1_default_price;
    agent2.get<cqspc::Wallet>() = 100 * good_1_default_price;

    // Add the goods
    agent1.get<cqspc::ResourceStockpile>()[good_1] = 100;

    cqspc::ResourceLedger ledger;
    ledger[good_1] = 100;
    // Then try to sell it
    ASSERT_TRUE(cqsp::core::actions::SellGood(agent1, ledger));
    ASSERT_TRUE(cqsp::core::actions::PurchaseGood(agent2, ledger));

    // Check if the good is sold then bought
    EXPECT_EQ(agent2.get<cqspc::ResourceStockpile>()[good_1], 100);
    EXPECT_EQ(agent1.get<cqspc::ResourceStockpile>()[good_1], 0);

    auto& market_comp = market.get<cqspc::Market>();
    EXPECT_EQ(market_comp[good_1].demand, 100);
    EXPECT_EQ(market_comp[good_1].supply, 100);

    // Then change the price
    cqsp::core::systems::SysMarket market_two_system(game);
    market_two_system.DoSystem();
    // Check the price
    // Price should be the same, because the amount was the same
    EXPECT_NEAR(market_comp[good_1].price, good_1_default_price, 5);
}

// If they buy less than what is on the market
// So there is an over supply the price should drop.
TEST_F(MarketTwoTest, BuySellOverSupplyTest) {
    // Add the resources and sell them, then try buying them, then get the change in price
    agent1.get<cqspc::Wallet>() = 1000 * good_1_default_price;
    agent2.get<cqspc::Wallet>() = 1000 * good_1_default_price;

    // Add the goods
    agent1.get<cqspc::ResourceStockpile>()[good_1] = 100;

    cqspc::ResourceLedger ledger;
    ledger[good_1] = 100;
    // Then try to sell it
    ASSERT_TRUE(cqsp::core::actions::SellGood(agent1, ledger));
    ledger[good_1] = 50;
    ASSERT_TRUE(cqsp::core::actions::PurchaseGood(agent2, ledger));

    // Check if the good is sold then bought
    EXPECT_EQ(agent1.get<cqspc::ResourceStockpile>()[good_1], 0);
    EXPECT_EQ(agent2.get<cqspc::ResourceStockpile>()[good_1], 50);

    auto& market_comp = market.get<cqspc::Market>();

    // Check market the resources
    EXPECT_EQ(market_comp[good_1].demand, 50);

    // Then change the price
    cqsp::core::systems::SysMarket market_two_system(game);
    market_two_system.DoSystem();

    // Check the price, lower price due to higher supply over demand
    EXPECT_LE(market_comp[good_1].price, good_1_default_price);
}
