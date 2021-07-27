/*
* Copyright 2021 Conquer Space
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