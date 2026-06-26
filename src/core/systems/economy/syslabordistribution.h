/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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

#include <unordered_map>
#include <vector>

#include "core/components/market.h"
#include "core/components/population.h"
#include "core/systems/economy/economyconfig.h"
#include "core/systems/isimulationsystem.h"

namespace cqsp::core::systems {
class SysLaborDistribution : public ISimulationSystem {
 public:
    explicit SysLaborDistribution(Game& game) : ISimulationSystem(game) {}
    void DoSystem() override;
    void Init() override;
    int Interval() const override { return ECONOMIC_TICK * 7; }

 private:
    void HandleJob(components::PopulationSegment& segment, components::Market& market);
    std::vector<components::GoodEntity> labor_goods;
    std::unordered_map<components::GoodEntity, entt::entity> good_labors;
    std::unordered_map<entt::entity, components::GoodEntity> entity_to_good;
};
}  // namespace cqsp::core::systems
