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

#include "common/systems/isimulationsystem.h"

namespace cqsp::common::systems {
class SysEconomy : public ISimulationSystem {
 public:
    void DoSystem(Game& game);

 private:
    void WalletReset(Universe& universe);
    void SysEmploymentHandler(Universe& universe);
    void SysResourceGenerator(Universe& universe);
    /// <summary>
    /// Creates goods that factories generate
    /// </summary>
    /// <param name="universe"></param>
    void SysProduction(Universe& universe);

    /// <summary>
    /// Creates demand for the system ResourceConsumption
    /// </summary>
    /// <param name="universe"></param>
    void SysDemandCreator(Universe& universe);

    /// <summary>
    /// Creates demand for factories
    /// </summary>
    /// <param name="universe"></param>
    void SysFactoryDemandCreator(Universe& universe);

    /// <summary>
    /// Sells all goods to the market
    /// </summary>
    /// <param name="universe"></param>
    void SysGoodSeller(Universe& universe);

    /// <summary>
    /// Resolves demand to supply, delivers goods to demand. Essentially it sells the goods.
    /// </summary>
    /// <param name="universe"></param>
    void SysDemandResolver(Universe& universe);

    /// <summary>
    /// Consumes resources for next production tick, and also verifies the conditions that this factory can
    /// generate resources, such as working population.
    /// </summary>
    /// <param name="universe"></param>
    void SysProductionStarter(Universe& universe);

    void SysPriceDetermine(Universe& universe);

    /// <summary>
    /// Consumes resource consumption.
    /// </summary>
    /// <param name="universe"></param>
    void SysConsumptionConsume(Universe& universe);

    void SysCommercialProcess(Universe& universe);
};
}  // namespace cqsp::common::systems
