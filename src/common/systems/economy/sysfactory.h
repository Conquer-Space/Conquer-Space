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
/// <summary>
/// Determines how much resources the factory decides to generate the next tick so that they don't
/// over produce when there is an insane excess of resources. This will create fluctuations
/// between production in the economy.
/// </summary>
class SysFactory : public ISimulationSystem {
 public:
    void DoSystem(Universe& universe);

 private:
    void SysFactoryProduction(Universe& universe);
    void SysMineProduction(Universe& universe);
    void SysFarmProduction(Universe& universe);
};
}  // namespace cqsp::common::systems
