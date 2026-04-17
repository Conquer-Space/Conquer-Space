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

#include "core/systems/economy/economyconfig.h"
#include "core/systems/isimulationsystem.h"

namespace cqsp::core::systems {
class SysProduction : public ISimulationSystem {
 public:
    explicit SysProduction(Game& game) : ISimulationSystem(game), economy_config(GetUniverse().economy_config) {}
    void DoSystem() override;
    int Interval() override { return ECONOMIC_TICK; }

 private:
    void ProcessIndustries(Node& node);
    void ScaleIndustry(Node& industry_node, components::Market& market);
    void ProcessIndustry(Node& industry_node, components::Market& market, Node& population_node, double infra_cost);
    void ScaleConstruction(Node& industry_node, double pl_ratio);
    bool HandleConstruction(Node& industry_node, components::Market& market);
    void IndustryFsm();
    components::IndustryState SteadyState(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState MaximumProduction(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState MinimumProduction(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState Construction(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState Demolishing(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState Shrinking(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState Expanding(entt::entity industry, components::ProductionUnit& production);
    components::IndustryState Shortage(entt::entity industry, components::ProductionUnit& production);
    void ProductionPreprocessing(entt::entity industry, components::ProductionUnit& production);

    double employed = 0;

    const EconomyConfig& economy_config;
};
}  // namespace cqsp::core::systems
