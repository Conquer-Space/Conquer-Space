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

#include <tracy/Tracy.hpp>

#include "client/components/clientctx.h"
#include "core/actions/maneuver/commands.h"
#include "core/actions/maneuver/transfers.h"
#include "core/actions/shiplaunchaction.h"
#include "core/components/bodies.h"
#include "core/components/colony.h"
#include "core/components/maneuver.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/components/orders.h"
#include "core/components/ships.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"
#include "sysspaceport.h"

namespace cqsp::core::systems {
void SysSpacePort::DoSystem() {
    ZoneScoped;
    auto space_ports = GetUniverse().view<components::infrastructure::SpacePort>();
    for (entt::entity space_port : space_ports) {
        ZoneScoped;
        auto& port_component = GetUniverse().get<components::infrastructure::SpacePort>(space_port);
        for (auto& [target, delivery_queue] : port_component.deliveries) {
            // Let's ignore the moon for now
            // Let's try to figure out the target
            entt::entity common_soi = commands::GetCommonSOI(GetUniverse(), port_component.reference_body, target);
            while (!delivery_queue.empty()) {
                components::infrastructure::TransportedGood& element = delivery_queue.back();
                // So this disables moons or something
                ProcessShippedGood(delivery_queue.back(), target, common_soi, port_component);
                // But we should also have non good stuff...
                delivery_queue.pop_back();
            }
        }
    }

    // then parse all cities with a docked ship
    // and also parse provinces with a docked ship as well
    for (auto&& [space_port, space_port_comp, docked_ships] :
         GetUniverse().view<components::infrastructure::SpacePort, components::DockedShips>().each()) {
        ProcessDockedShips(space_port);
    }

    for (auto&& [space_port, docked_ships, province_comp] :
         GetUniverse().view<components::DockedShips, components::Province>().each()) {
        std::vector<entt::entity> to_remove;
        for (entt::entity ship : docked_ships.docked_ships) {
            // Now unload the resources in the space port
            if (GetUniverse().any_of<components::ships::CargoHold>(ship)) {
                auto& cargo = GetUniverse().get<components::ships::CargoHold>(ship);
                // TODO: Check cargo
                if (GetUniverse().valid(space_port) &&
                    GetUniverse().any_of<components::ColonizationTarget>(space_port)) {
                    auto& target = GetUniverse().get<components::ColonizationTarget>(space_port);
                    // Progress target
                    switch (target.steps) {
                        case components::ColonizationSteps::Surveying:
                            target.steps = components::ColonizationSteps::Preparation;
                            break;
                        case components::ColonizationSteps::Preparation:
                            target.steps = components::ColonizationSteps::InitialBase;
                            break;
                        case components::ColonizationSteps::InitialBase:
                            target.steps = components::ColonizationSteps::HumanSettlement;
                            break;
                        case components::ColonizationSteps::HumanSettlement:
                            target.steps = components::ColonizationSteps::PermanentSettlement;
                            break;
                        case components::ColonizationSteps::PermanentSettlement:
                            // Now we own the province?
                            province_comp.country = target.colonizer;
                            GetUniverse().remove<components::ColonizationTarget>(space_port);
                    }
                    // Once again rust would actually be great for this
                    GetUniverse().remove<components::ships::CargoHold>(ship);
                }
            }
        }
    }
}

void SysSpacePort::ProcessDockedShips(entt::entity space_port) {
    ZoneScoped;
    if (!GetUniverse().any_of<components::DockedShips>(space_port)) {
        return;
    }
    auto& docked_ships = GetUniverse().get<components::DockedShips>(space_port);
    auto& space_port_comp = GetUniverse().get<components::infrastructure::SpacePort>(space_port);
    // Check for each of the docked ships
    for (entt::entity ship : docked_ships.docked_ships) {
        // Now unload the resources in the space port
        if (GetUniverse().any_of<components::ships::CargoHold>(ship)) {
            auto& cargo = GetUniverse().get<components::ships::CargoHold>(ship);
            // TODO: Check cargo
            auto& city_comp = GetUniverse().get<components::City>(space_port);
            if (GetUniverse().valid(city_comp.province) &&
                GetUniverse().any_of<components::ColonizationTarget>(city_comp.province)) {
                auto& target = GetUniverse().get<components::ColonizationTarget>(city_comp.province);
                // Progress target
                switch (target.steps) {
                    case components::ColonizationSteps::Surveying:
                        target.steps = components::ColonizationSteps::Preparation;
                        break;
                    case components::ColonizationSteps::Preparation:
                        target.steps = components::ColonizationSteps::InitialBase;
                        break;
                    case components::ColonizationSteps::InitialBase:
                        target.steps = components::ColonizationSteps::HumanSettlement;
                        break;
                    case components::ColonizationSteps::HumanSettlement:
                        target.steps = components::ColonizationSteps::PermanentSettlement;
                        break;
                    case components::ColonizationSteps::PermanentSettlement:
                        // No longer a colonization target so we remove that component and then also do stuff
                        GetUniverse().remove<components::ColonizationTarget>(city_comp.province);
                        // Please don't access the target after this
                        // rust would actually fix this
                }
            }
        }
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
    entt::entity land_target = element.target_province;
    if (element.target_province == entt::null) {
        auto& cities = GetUniverse().get<components::Settlements>(target);
        land_target = cities.settlements.front();
    }
    auto& cities = GetUniverse().get<components::Settlements>(target);
    commands::LandOnMoon(GetUniverse(), ship, target, land_target);
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
    if (element.target_province == entt::null) {
        auto& cities = GetUniverse().get<components::Settlements>(target);
        GetUniverse().emplace<components::OrbitEntityTarget>(dock_city, cities.settlements.front());
    } else {
        GetUniverse().emplace<components::OrbitEntityTarget>(dock_city, element.target_province);
    }
    GetUniverse().emplace<components::Trigger>(dock_city, components::Trigger::OnCrash);
    GetUniverse().emplace<components::Command>(dock_city, components::Command::LandOnBody);
    command_queue.commands.push_back(dock_city);

    return ship;
}

void SysSpacePort::ProcessShippedGood(const components::infrastructure::TransportedGood& element, entt::entity target,
                                      entt::entity common_soi, components::infrastructure::SpacePort& port_component) {
    ZoneScoped;
    entt::entity ship = entt::null;
    if (common_soi == target) {
        // Returning from moon
        ship = ReturnFromMoonManeuver(element, port_component.reference_body, target);
    } else if (common_soi == port_component.reference_body) {
        // Target moon
        ship = TargetMoonManeuver(element, port_component.reference_body, target);
    } else {
        SPDLOG_ERROR("Interplanetary transfers not supported yet");
    }

    if (ship != entt::null) {
        // We transfer to the body
        if (GetUniverse().good_map.contains(element.good)) {
            auto& stockpile = GetUniverse().emplace<components::ResourceStockpile>(ship);
            stockpile[GetUniverse().good_map[element.good]] = element.amount;
        } else {
            auto& cargo = GetUniverse().emplace<components::ships::CargoHold>(ship);
            cargo.cargo.push_back(element.good);
        }
        GetUniverse().emplace<client::ctx::VisibleOrbit>(ship);
    }
}
}  // namespace cqsp::core::systems
