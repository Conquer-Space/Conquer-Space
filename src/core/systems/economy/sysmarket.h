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

#include "core/components/market.h"
#include "core/components/resource.h"
#include "core/systems/economy/economyconfig.h"
#include "core/systems/isimulationsystem.h"

namespace cqsp::core::systems {
class SysMarket : public ISimulationSystem {
 public:
    explicit SysMarket(Game& game) : ISimulationSystem(game) {}
    void DoSystem() override;
    int Interval() override { return ECONOMIC_TICK; }

    void Init() override;

 private:
    void DeterminePrice(components::Market& market, Node& good_entity);
    void DetermineShortages(components::Market& market);

    components::ResourceMap base_prices;
};
}  // namespace cqsp::core::systems
