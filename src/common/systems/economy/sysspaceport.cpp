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
#include "common/systems/economy/sysspaceport.h"

#include "client/components/clientctx.h"
#include "common/actions/maneuver/commands.h"
#include "common/actions/maneuver/transfers.h"
#include "common/actions/shiplaunchaction.h"
#include "common/components/bodies.h"
#include "common/components/maneuver.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/orders.h"
#include "common/components/spaceport.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"

namespace cqsp::common::systems {
void SysSpacePort::DoSystem() {
    auto space_ports = GetUniverse().nodes<components::infrastructure::SpacePort>();
    for (Node space_port : space_ports) {
        auto& port_component = space_port.get<components::infrastructure::SpacePort>();
        for (auto& [target, delivery_queue] : port_component.deliveries) {

            // Let's ignore the moon for now
            // Let's try to figure out the target
            Node reference_body(space_port, port_component.reference_body);
            Node target(space_port, target);
            entt::entity common_soi = commands::GetCommonSOI(reference_body, target);
            while (!delivery_queue.empty()) {
                entt::entity ship = entt::null;
                components::infrastructure::TransportedGood& element = delivery_queue.back();
                if (common_soi == target) {
                    // Returning from moon
                    ship = ReturnFromMoonManeuver(element, reference_body, target);
                } else if (common_soi == port_component.reference_body) {
                    // Target moon
                    ship = TargetMoonManeuver(element, reference_body, target);
                } else {
                    SPDLOG_INFO("Interplanetary transfers not supported yet");
                }

                if (ship != entt::null) {
                    auto& stockpile = GetUniverse().emplace<components::ResourceStockpile>(ship);
                    stockpile[element.good] = element.amount;

                    GetUniverse().emplace<client::ctx::VisibleOrbit>(ship);
                }
                delivery_queue.pop_back();
            }
        }
        ProcessDockedShips(space_port);
    }
}

void SysSpacePort::ProcessDockedShips(Node& space_port) {
    if (!space_port.any_of<components::DockedShips>()) {
        return;
    }
    auto& docked_ships = space_port.get<components::DockedShips>();
    auto& space_port_comp = space_port.get<components::infrastructure::SpacePort>();
    // Check for each of the docked ships
    for (Node ship : space_port.Convert(docked_ships.docked_ships)) {
        // Now unload the resources in the space port
        if (!ship.any_of<components::ResourceStockpile>()) {
            continue;
        }
        space_port_comp.output_resources += ship.get<components::ResourceStockpile>();
        ship.remove<components::ResourceStockpile>();
    }
}

Node SysSpacePort::TargetMoonManeuver(const components::infrastructure::TransportedGood& element,
                                      Node& reference_body, Node& target) {
    // Spawn new ship and send it to a parking orbit
    // Get the reference body
    auto& body = reference_body.get<components::bodies::Body>();
    // Let's set a parking orbit at 8% of the planet's radius
    // TODO(EhWhoAmI): It should be a factor of the atmosphere in the future
    const double eccentricity = std::fabs(GetUniverse().random->GetRandomNormal(0, 0.000005));
    const double semi_major_axis = GetUniverse().random->GetRandomNormal(body.radius * 1.08, body.radius * 0.01);
    const double inclination = GetUniverse().random->GetRandomNormal(0, 0.05);
    components::types::Orbit source_orbit(semi_major_axis, eccentricity, inclination, 0, 0.1, 0, reference_body);
    Node ship = common::actions::LaunchShip(GetUniverse(), source_orbit);
    GetUniverse().emplace<components::Name>(
        ship, fmt::format("{} Transport Vehicle", util::GetName(GetUniverse(), element.good)));
    // Then target the target body
    // Land on armstrong
    auto& cities = GetUniverse().get<components::Habitation>(target);
    Node first_city(GetUniverse(), cities.settlements.front());
    commands::LandOnMoon(ship, target, first_city);
    return ship;
}

Node SysSpacePort::ReturnFromMoonManeuver(const components::infrastructure::TransportedGood& element,
                                          Node& reference_body, Node& target) {
    // Spawn new ship and send it to a parking orbit
    // Get the reference body
    auto& body = GetUniverse().get<components::bodies::Body>(reference_body);
    // Let's set a parking orbit at 8% of the planet's radius
    // TODO(EhWhoAmI): It should be a factor of the atmosphere in the future
    const double eccentricity = std::fabs(GetUniverse().random->GetRandomNormal(0, 0.000005));
    const double semi_major_axis = GetUniverse().random->GetRandomNormal(body.radius * 1.08, body.radius * 0.01);
    const double inclination = GetUniverse().random->GetRandomNormal(0, 0.05);
    components::types::Orbit source_orbit(semi_major_axis, eccentricity, inclination, 0, 0.1, 0, reference_body);
    Node ship = common::actions::LaunchShip(GetUniverse(), source_orbit);
    ship.emplace<components::Name>(fmt::format("{} Transport Vehicle", util::GetName(GetUniverse(), element.good)));
    // Then target the target body
    // Land on armstrong

    auto& target_body = target.get<components::bodies::Body>();

    systems::commands::LeaveSOI(ship, target_body.radius * 5);
    systems::commands::PushManeuver(ship, systems::commands::MakeManeuver(glm::dvec3(0, 0, 0), 1000));

    auto& command_queue = ship.get_or_emplace<components::CommandQueue>();

    // Circularize
    Node circularize(GetUniverse());
    circularize.emplace<components::Trigger>(components::Trigger::OnExitSOI);
    circularize.emplace<components::Command>(components::Command::CircularizeAtPeriapsis);
    command_queue.commands.push_back(circularize);

    // Reenter
    Node reenter(GetUniverse());
    reenter.emplace<components::Trigger>(components::Trigger::OnManeuver);
    reenter.emplace<components::Command>(components::Command::SetPeriapsis);
    reenter.emplace<components::OrbitScalar>(target_body.radius * 0.9);
    command_queue.commands.push_back(reenter);

    Node dock_city(GetUniverse());
    auto& cities = GetUniverse().get<components::Habitation>(target);
    dock_city.emplace<components::Trigger>(components::Trigger::OnCrash);
    dock_city.emplace<components::Command>(components::Command::LandOnBody);
    dock_city.emplace<components::OrbitEntityTarget>(cities.settlements.front());
    command_queue.commands.push_back(dock_city);

    return ship;
}
}  // namespace cqsp::common::systems
