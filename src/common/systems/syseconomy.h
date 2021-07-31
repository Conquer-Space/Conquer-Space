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

namespace conquerspace::common::systems {
class SysFactory : public ISimulationSystem {
   public:
    void DoSystem(components::Universe& universe);

   private:
    void SysResourceGenerator(components::Universe& universe);
    void SysProduction(components::Universe& universe);

    /// <summary>
    /// Creates demand for the system ResourceConsumption
    /// </summary>
    /// <param name="universe"></param>
    void SysConsumption(components::Universe& universe);

    /// <summary>
    /// Creates demand for the factory
    /// </summary>
    /// <param name="universe"></param>
    void SysFactoryDemandCreator(components::Universe& universe);

    /// <summary>
    /// Sells all goods to the market
    /// </summary>
    /// <param name="universe"></param>
    void SysGoodSeller(components::Universe& universe);

    /// <summary>
    /// Gives the demand to the supply
    /// </summary>
    /// <param name="universe"></param>
    void SysDemandResolver(components::Universe& universe);

    /// <summary>
    /// Consumes resources for next production tick
    /// </summary>
    /// <param name="universe"></param>
    void SysProductionStarter(components::Universe& universe);
};
}