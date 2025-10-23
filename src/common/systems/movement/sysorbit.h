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

#include <vector>

#include "common/components/orbit.h"
#include "common/systems/isimulationsystem.h"

namespace cqsp::common::systems {
class SysOrbit : public ISimulationSystem {
 public:
    explicit SysOrbit(Game& game) : ISimulationSystem(game) {}
    void DoSystem() override;
    int Interval() override { return 1; }

 private:
    void ParseOrbitTree(entt::entity parent, entt::entity body);

    /// <summary>
    /// Sets the SOI of the entity to the parent
    /// </summary>
    /// <param name="universe"></param>
    /// <param name="body">Needs to have a Body and Orbit parameter</param>
    /// <param name="ppos">Parent position</param>
    void LeaveSOI(const entt::entity& body, entt::entity& parent, components::types::Orbit& orb,
                  components::types::Kinematics& pos, components::types::Kinematics& p_pos);

    /// <summary>
    /// Change the current body's SOI into a child SOI
    /// </summary>
    /// <param name="universe"></param>
    /// <param name="parent"></param>
    /// <param name="body">Body that we want to check if it's entering a SOI</param>
    bool EnterSOI(const entt::entity& parent, const entt::entity& body);

    /// <summary>
    /// Check if the entity has crashed into its parent object
    /// </summary>
    /// <param name="universe"></param>
    /// <param name="orb"></param>
    /// <param name="body"></param>
    /// <param name="parent"></param>
    void CrashObject(components::types::Orbit& orb, entt::entity body, entt::entity parent);

    void UpdateCommandQueue(components::types::Orbit& orb, entt::entity body, entt::entity parent);

    void CalculateImpulse(components::types::Orbit& orb, entt::entity body, entt::entity parent);
};

class SysPath : public ISimulationSystem {
 public:
    explicit SysPath(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
    int Interval() { return 1; }
};

class SysSurface : public ISimulationSystem {
 public:
    explicit SysSurface(Game& game) : ISimulationSystem(game) {}
    void DoSystem();
    int Interval() { return 1; }
};
}  // namespace cqsp::common::systems
