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
#include <gtest/gtest.h>

#include <iostream>

#include "common/components/economy.h"
#include "common/systems/economy/markethelpers.h"
#include "common/systems/economy/sysmarket.h"
#include "common/universe.h"

namespace cqspc = cqsp::common::components;

class MarketTwoTest : public ::testing::Test {
 protected:
    MarketTwoTest() : universe(game.GetUniverse()) {}

    void SetUp() override {
        good_1 = universe.create();
        good_2 = universe.create();

        market = cqsp::common::systems::economy::CreateMarket(universe);
        auto& market_comp = universe.get<cqspc::Market>(market);
        market_comp[good_1].price = good_1_default_price;
        market_comp[good_2].price = good_2_default_price;

        // Setup agents
        agent1 = universe.create();
        universe.emplace<cqspc::Wallet>(agent1);
        universe.emplace<cqspc::ResourceStockpile>(agent1);
        universe.emplace<cqspc::MarketAgent>(agent1, market);

        agent2 = universe.create();
        universe.emplace<cqspc::Wallet>(agent2);
        universe.emplace<cqspc::ResourceStockpile>(agent2);
        universe.emplace<cqspc::MarketAgent>(agent2, market);
    }

    cqsp::common::Game game;
    cqsp::common::Universe& universe;
    entt::entity agent1;
    entt::entity agent2;
    entt::entity market;
    entt::entity good_1;
    const int good_1_default_price = 100;
    entt::entity good_2;
    const int good_2_default_price = 300;
};

TEST_F(MarketTwoTest, SellTest) {
    // Then test if the prices fluctuate or something
    // Add resources
    auto& stockpile = universe.get<cqspc::ResourceStockpile>(agent1);
    stockpile[good_1] = 100;
    // Now test sell the goods
    ASSERT_TRUE(cqsp::common::systems::economy::SellGood(universe, agent1, stockpile));
    // Then check if the goods are sold
    auto& market_comp = universe.get<cqspc::Market>(market);

    // Check if goods are on the market
    EXPECT_EQ(market_comp[good_1].supply, 100);

    // Check if agent sold resources
    EXPECT_EQ(stockpile[good_1], 0);
    // So the supply is 100 now
    // Check if wallet is subtracted
    auto& wallet = universe.get<cqspc::Wallet>(agent1);
    EXPECT_EQ(wallet.GetBalance(), good_1_default_price * 100);
}

TEST_F(MarketTwoTest, BuyTest) {
    // Add money to the wallet so that they have enough to buy
    universe.get<cqspc::Wallet>(agent1) = 100 * good_1_default_price;

    // Now test sell the goods
    cqspc::ResourceStockpile to_buy;
    to_buy[good_1] = 100;
    ASSERT_TRUE(cqsp::common::systems::economy::PurchaseGood(universe, agent1, to_buy));

    auto& market_comp = universe.get<cqspc::Market>(market);

    // Check demand
    EXPECT_EQ(market_comp[good_1].demand, 100);

    // Check wallet
    EXPECT_EQ(universe.get<cqspc::Wallet>(agent1).GetBalance(), 0);
}

TEST_F(MarketTwoTest, BuySellTest) {
    // Add the resources and sell them, then try buying them, then get the change in price
    universe.get<cqspc::Wallet>(agent1) = 100 * good_1_default_price;
    universe.get<cqspc::Wallet>(agent2) = 100 * good_1_default_price;

    // Add the goods
    universe.get<cqspc::ResourceStockpile>(agent1)[good_1] = 100;

    cqspc::ResourceLedger ledger;
    ledger[good_1] = 100;
    // Then try to sell it
    ASSERT_TRUE(cqsp::common::systems::economy::SellGood(universe, agent1, ledger));
    ASSERT_TRUE(cqsp::common::systems::economy::PurchaseGood(universe, agent2, ledger));

    // Check if the good is sold then bought
    EXPECT_EQ(universe.get<cqspc::ResourceStockpile>(agent2)[good_1], 100);
    EXPECT_EQ(universe.get<cqspc::ResourceStockpile>(agent1)[good_1], 0);

    auto& market_comp = universe.get<cqspc::Market>(market);
    EXPECT_EQ(market_comp[good_1].demand, 100);
    EXPECT_EQ(market_comp[good_1].supply, 100);

    // Then change the price
    cqsp::common::systems::SysMarket market_two_system(game);
    market_two_system.DoSystem();
    // Check the price
    // Price should be the same, because the amount was the same
    EXPECT_NEAR(market_comp[good_1].price, good_1_default_price, 5);
}

// If they buy less than what is on the market
// So there is an over supply the price should drop.
TEST_F(MarketTwoTest, BuySellOverSupplyTest) {
    // Add the resources and sell them, then try buying them, then get the change in price
    universe.get<cqspc::Wallet>(agent1) = 1000 * good_1_default_price;
    universe.get<cqspc::Wallet>(agent2) = 1000 * good_1_default_price;

    // Add the goods
    universe.get<cqspc::ResourceStockpile>(agent1)[good_1] = 100;

    cqspc::ResourceLedger ledger;
    ledger[good_1] = 100;
    // Then try to sell it
    ASSERT_TRUE(cqsp::common::systems::economy::SellGood(universe, agent1, ledger));
    ledger[good_1] = 50;
    ASSERT_TRUE(cqsp::common::systems::economy::PurchaseGood(universe, agent2, ledger));

    // Check if the good is sold then bought
    EXPECT_EQ(universe.get<cqspc::ResourceStockpile>(agent1)[good_1], 0);
    EXPECT_EQ(universe.get<cqspc::ResourceStockpile>(agent2)[good_1], 50);

    auto& market_comp = universe.get<cqspc::Market>(market);

    // Check market the resources
    EXPECT_EQ(market_comp[good_1].demand, 50);

    // Then change the price
    cqsp::common::systems::SysMarket market_two_system(game);
    market_two_system.DoSystem();

    // Check the price, lower price due to higher supply over demand
    EXPECT_LE(market_comp[good_1].price, good_1_default_price);
}
