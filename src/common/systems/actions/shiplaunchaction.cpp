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
#include "common/systems/actions/shiplaunchaction.h"

#include <fmt/format.h>
#include <string>



#include "common/components/coordinates.h"
#include "common/components/ships.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/organizations.h"



entt::entity cqsp::common::systems::actions::CreateShip(
    cqsp::common::Universe& universe, entt::entity civEnt, const glm::vec3& orbit,
    entt::entity starsystem) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspb = cqsp::common::components::bodies;
    entt::entity ship = universe.create();
    universe.emplace<cqsps::Ship>(ship);

    auto &position = universe.emplace<cqspt::Kinematics>(ship);

    // Get planet position
    position.position = orbit;
    universe.get<cqspb::StarSystem>(starsystem).bodies.push_back(ship);
    // Set name
    universe.emplace<components::Name>(ship, fmt::format("Ship {}", ship));
    return ship;
}

entt::entity cqsp::common::systems::actions::CreateShip(
    cqsp::common::Universe& universe, entt::entity civ, entt::entity orbit,
    entt::entity starsystem) {
    namespace cqspt = cqsp::common::components::types;
    return CreateShip(universe, civ, cqspt::toVec3(universe.get<cqspt::Orbit>(orbit)), starsystem);
}

entt::entity cqsp::common::systems::actions::CreateShip(
    cqsp::common::Universe& universe, entt::entity fleetEnt,
    entt::entity starsystemEnt, const glm::vec3& orbit,
    const std::string& shipName) {

    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspb = cqsp::common::components::bodies;
    entt::entity ship = universe.create();
    universe.emplace<cqsps::Ship>(ship);

    auto& position = universe.emplace<cqspt::Kinematics>(ship);
    // Get planet position
    position.position = orbit;
    universe.get<cqspb::StarSystem>(starsystemEnt).bodies.push_back(ship);
    // Set name
    universe.emplace<components::Name>(ship, shipName);
    // Set in fleet
    universe.get<components::ships::Fleet>(fleetEnt).ships.push_back(ship);


    return ship;
}

entt::entity cqsp::common::systems::actions::CreateShip(
    cqsp::common::Universe& universe, entt::entity fleetEnt,
    entt::entity starsystemEnt, entt::entity orbitEnt,
    const std::string& shipName) {
    namespace cqspt = cqsp::common::components::types;
    return CreateShip(universe, fleetEnt, starsystemEnt,
               cqspt::toVec3(universe.get<cqspt::Orbit>(orbitEnt)), shipName);
}

