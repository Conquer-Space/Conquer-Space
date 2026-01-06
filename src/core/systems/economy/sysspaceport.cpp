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
#include "core/systems/economy/sysspaceport.h"

#include "client/components/clientctx.h"
#include "core/actions/maneuver/commands.h"
#include "core/actions/maneuver/transfers.h"
#include "core/actions/shiplaunchaction.h"
#include "core/components/bodies.h"
#include "core/components/maneuver.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/components/orders.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"

namespace cqsp::core::systems {
void SysSpacePort::DoSystem() {
    auto space_ports = GetUniverse().view<components::infrastructure::SpacePort>();
    for (entt::entity space_port : space_ports) {
        auto& port_component = GetUniverse().get<components::infrastructure::SpacePort>(space_port);
        for (auto& [target, delivery_queue] : port_component.deliveries) {
            // Let's ignore the moon for now
            // Let's try to figure out the target
            entt::entity common_soi = commands::GetCommonSOI(GetUniverse(), port_component.reference_body, target);
            while (!delivery_queue.empty()) {
                entt::entity ship = entt::null;
                components::infrastructure::TransportedGood& element = delivery_queue.back();
                if (common_soi == target) {
                    // Returning from moon
                    ship = ReturnFromMoonManeuver(element, port_component.reference_body, target);
                } else if (common_soi == port_component.reference_body) {
                    // Target moon
                    ship = TargetMoonManeuver(element, port_component.reference_body, target);
                } else {
                    SPDLOG_INFO("Interplanetary transfers not supported yet");
                }

                if (ship != entt::null) {
                    auto& stockpile = GetUniverse().emplace<components::ResourceStockpile>(ship);
                    stockpile[GetUniverse().good_map[element.good]] = element.amount;

                    GetUniverse().emplace<client::ctx::VisibleOrbit>(ship);
                }
                delivery_queue.pop_back();
            }
        }
        ProcessDockedShips(space_port);
    }
}

void SysSpacePort::ProcessDockedShips(entt::entity space_port) {
    if (!GetUniverse().any_of<components::DockedShips>(space_port)) {
        return;
    }
    auto& docked_ships = GetUniverse().get<components::DockedShips>(space_port);
    auto& space_port_comp = GetUniverse().get<components::infrastructure::SpacePort>(space_port);
    // Check for each of the docked ships
    for (entt::entity ship : docked_ships.docked_ships) {
        // Now unload the resources in the space port
        if (!GetUniverse().any_of<components::ResourceStockpile>(ship)) {
            continue;
        }
        space_port_comp.output_resources += GetUniverse().get<components::ResourceStockpile>(ship);
        GetUniverse().remove<components::ResourceStockpile>(ship);
    }
}

entt::entity SysSpacePort::TargetMoonManeuver(const components::infrastructure::TransportedGood& element,
                                              entt::entity reference_body, entt::entity target) {
    // Spawn new ship and send it to a parking orbit
    // Get the reference body
    auto& body = GetUniverse().get<components::bodies::Body>(reference_body);
    // Let's set a parking orbit at 8% of the planet's radius
    // TODO(EhWhoAmI): It should be a factor of the atmosphere in the future
    const double eccentricity = std::fabs(GetUniverse().random->GetRandomNormal(0, 0.000005));
    const double semi_major_axis = GetUniverse().random->GetRandomNormal(body.radius * 1.08, body.radius * 0.01);
    const double inclination = GetUniverse().random->GetRandomNormal(0, 0.05);
    components::types::Orbit source_orbit(semi_major_axis, eccentricity, inclination, 0, 0.1, 0, reference_body);
    entt::entity ship = core::actions::LaunchShip(GetUniverse(), source_orbit);
    GetUniverse().emplace<components::Name>(
        ship, fmt::format("{} Transport Vehicle", util::GetName(GetUniverse(), element.good)));
    // Then target the target body
    // Land on armstrong
    auto& cities = GetUniverse().get<components::Settlements>(target);
    commands::LandOnMoon(GetUniverse(), ship, target, cities.settlements.front());
    return ship;
}

entt::entity SysSpacePort::ReturnFromMoonManeuver(const components::infrastructure::TransportedGood& element,
                                                  entt::entity reference_body, entt::entity target) {
    // Spawn new ship and send it to a parking orbit
    // Get the reference body
    auto& body = GetUniverse().get<components::bodies::Body>(reference_body);
    // Let's set a parking orbit at 8% of the planet's radius
    // TODO(EhWhoAmI): It should be a factor of the atmosphere in the future
    const double eccentricity = std::fabs(GetUniverse().random->GetRandomNormal(0, 0.000005));
    const double semi_major_axis = GetUniverse().random->GetRandomNormal(body.radius * 1.08, body.radius * 0.01);
    const double inclination = GetUniverse().random->GetRandomNormal(0, 0.05);
    components::types::Orbit source_orbit(semi_major_axis, eccentricity, inclination, 0, 0.1, 0, reference_body);
    entt::entity ship = core::actions::LaunchShip(GetUniverse(), source_orbit);
    GetUniverse().emplace<components::Name>(
        ship, fmt::format("{} Transport Vehicle", util::GetName(GetUniverse(), element.good)));
    // Then target the target body
    // Land on armstrong

    auto& target_body = GetUniverse().get<components::bodies::Body>(target);

    systems::commands::LeaveSOI(GetUniverse(), ship, target_body.radius * 5);
    systems::commands::PushManeuver(GetUniverse(), ship, systems::commands::MakeManeuver(glm::dvec3(0, 0, 0), 1000));

    auto& command_queue = GetUniverse().get_or_emplace<components::CommandQueue>(ship);

    // Circularize
    entt::entity circularize = GetUniverse().create();
    GetUniverse().emplace<components::Trigger>(circularize, components::Trigger::OnExitSOI);
    GetUniverse().emplace<components::Command>(circularize, components::Command::CircularizeAtPeriapsis);
    command_queue.commands.push_back(circularize);

    // Reenter
    entt::entity reenter = GetUniverse().create();
    GetUniverse().emplace<components::Trigger>(reenter, components::Trigger::OnManeuver);
    GetUniverse().emplace<components::Command>(reenter, components::Command::SetPeriapsis);
    GetUniverse().emplace<components::OrbitScalar>(reenter, target_body.radius * 0.9);
    command_queue.commands.push_back(reenter);

    entt::entity dock_city = GetUniverse().create();
    auto& cities = GetUniverse().get<components::Settlements>(target);
    GetUniverse().emplace<components::Trigger>(dock_city, components::Trigger::OnCrash);
    GetUniverse().emplace<components::Command>(dock_city, components::Command::LandOnBody);
    GetUniverse().emplace<components::OrbitEntityTarget>(dock_city, cities.settlements.front());
    command_queue.commands.push_back(dock_city);

    return ship;
}
}  // namespace cqsp::core::systems
