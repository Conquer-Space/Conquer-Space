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

#include <string>

#include <entt/entt.hpp>
#include <glm/vec3.hpp>

#include "common/components/coordinates.h"
#include "common/components/orbit.h"
#include "common/universe.h"

namespace cqsp::common::actions {
// deprecated
entt::entity CreateShip(Universe &universe, entt::entity civ, const glm::vec3 &orbit, entt::entity starsystem);
// deprecated
entt::entity CreateShip(Universe &universe, entt::entity civ, entt::entity orbit, entt::entity starsystem);
/**
 *@brief creates a ship with a name and assigns it to a fleet and solar system
 *@param universe The registry for the entities
 *@param fleetEnt The fleet to be used
 *@param starsystemEnt The star system where the ship is created in
 *@return The ship that is created
 */
entt::entity CreateShip(Universe &universe, entt::entity fleetEnt, entt::entity starsystemEnt, const glm::vec3 &orbit,
                        const std::string &shipName);
//@brief just like createShip with fleets but uses an orbit entity instead
entt::entity CreateShip(Universe &universe, entt::entity fleetEnt, entt::entity starsystemEnt, entt::entity orbitEnt,
                        const std::string &shipName);

entt::entity LaunchShip(Universe &universe, const components::types::Orbit &orbit);
}  // namespace cqsp::common::actions
