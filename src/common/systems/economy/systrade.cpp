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
#include "common/systems/economy/systrade.h"

#include "common/components/economy.h"
#include "common/components/surface.h"

using cqsp::common::systems::SysTrade;
namespace components = cqsp::common::components;
using components::Market;
using components::PlanetaryMarket;
using components::Habitation;
using entt::entity;

void SysTrade::DoSystem() {
    // Sort through all the districts, and figure out their trade
    // Get all the markets
    // Then cross reference to see if they can buy or sell
    // Then list all the markets
    // Get the market of the planet, and add latent supply and demand, and then compute the market
    for (entity tentity : GetUniverse().view<Market, PlanetaryMarket, Habitation>()) {
        auto& p_market = GetUniverse().get<Market>(tentity);
        auto& habitation = GetUniverse().get<Habitation>(tentity);
        for (entity habitation : habitation.settlements) {
            if (!GetUniverse().any_of<Market>(habitation)) {
                continue;
            }
            auto& market = GetUniverse().get<Market>(habitation);
            p_market.supply += market.latent_supply;
            p_market.demand += market.latent_demand;
        }
    }
}
