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

Node CreateShip(Node& starsystem, Node& fleet, const glm::vec3& orbit, const std::string& shipName) {
    Node ship(fleet.universe());
    ship.emplace<Ship>();

    auto& position = ship.emplace<types::Kinematics>();
    // Get planet position
    position.position = orbit;
    //universe.get<cqspb::StarSystem>(starsystemEnt).bodies.push_back(ship);
    // Set name
    if (shipName.empty()) {
        ship.emplace<components::Name>(fmt::format("Ship {}", ship));

    } else {
        ship.emplace<components::Name>(shipName);
    }
    // Set in fleet
    if (fleet.all_of<components::ships::Fleet>()) fleet.get<components::ships::Fleet>().ships.push_back(ship);

    return ship;
}

Node CreateShip(Node& starsystem, Node& fleet, Node& orbit, const std::string& shipName) {
    return CreateShip(starsystem, fleet, orbit.get<types::Orbit>(), shipName);
}

Node CreateShip(Node& starsystem, Node& fleet, const components::types::Orbit& orbit, const std::string& shipName) {
    return CreateShip(starsystem, fleet, types::toVec3AU(orbit), shipName);
}

Node LaunchShip(Universe& universe, const components::types::Orbit& orbit) {
    Node ship(universe);
    ship.emplace<Ship>();
    // Now do things
    auto& o = ship.emplace<types::Orbit>(orbit);
    ship.emplace<types::Kinematics>();
    Node orbiting_body(universe, orbit.reference_body);
    auto& body = orbiting_body.get<bodies::Body>();
    o.GM = body.GM;
    orbiting_body.get<bodies::OrbitalSystem>().push_back(ship);
    return ship;
}
}  // namespace cqsp::common::actions
