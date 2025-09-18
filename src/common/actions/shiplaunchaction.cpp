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
#include "common/actions/shiplaunchaction.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <string>

#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/ships.h"

namespace cqsp::common::actions {

namespace types = components::types;
namespace ships = components::ships;
namespace bodies = components::bodies;

using ships::Ship;

entt::entity CreateShip(Universe& universe, entt::entity civEnt, const glm::vec3& orbit, entt::entity starsystem) {
    entt::entity ship = universe.create();
    universe.emplace<Ship>(ship);

    auto& position = universe.emplace<types::Kinematics>(ship);

    // Get planet position
    position.position = orbit;
    //universe.get<cqspb::StarSystem>(starsystem).bodies.push_back(ship);
    // Set name
    universe.emplace<components::Name>(ship, fmt::format("Ship {}", ship));
    return ship;
}

entt::entity CreateShip(Universe& universe, entt::entity civ, entt::entity orbit, entt::entity starsystem) {
    return CreateShip(universe, civ, types::toVec3AU(universe.get<types::Orbit>(orbit)), starsystem);
}

entt::entity CreateShip(Universe& universe, entt::entity fleetEnt, entt::entity starsystemEnt, const glm::vec3& orbit,
                        const std::string& shipName) {
    entt::entity ship = universe.create();
    universe.emplace<Ship>(ship);

    auto& position = universe.emplace<types::Kinematics>(ship);
    // Get planet position
    position.position = orbit;
    //universe.get<cqspb::StarSystem>(starsystemEnt).bodies.push_back(ship);
    // Set name
    universe.emplace<components::Name>(ship, shipName);
    // Set in fleet
    universe.get<components::ships::Fleet>(fleetEnt).ships.push_back(ship);

    return ship;
}

entt::entity CreateShip(Universe& universe, entt::entity fleetEnt, entt::entity starsystemEnt, entt::entity orbitEnt,
                        const std::string& shipName) {
    return CreateShip(universe, fleetEnt, starsystemEnt, types::toVec3AU(universe.get<types::Orbit>(orbitEnt)),
                      shipName);
}

entt::entity LaunchShip(Universe& universe, components::types::Orbit& orbit) {
    // Set the orbit

    entt::entity ship = universe.create();
    universe.emplace<Ship>(ship);
    // Now do things
    auto& o = universe.emplace<types::Orbit>(ship, orbit);
    universe.emplace<types::Kinematics>(ship);
    auto& body = universe.get<bodies::Body>(orbit.reference_body);
    o.GM = body.GM;
    universe.get<bodies::OrbitalSystem>(orbit.reference_body).push_back(ship);
    return ship;
}
}  // namespace cqsp::common::actions
