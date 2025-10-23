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

#include "common/actions/maneuver/commands.h"
#include "common/actions/shiplaunchaction.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/spaceport.h"
#include "common/util/nameutil.h"

namespace cqsp::common::systems {
void SysSpacePort::DoSystem() {
    auto space_ports = GetUniverse().view<components::infrastructure::SpacePort>();
    for (entt::entity space_port : space_ports) {
        auto& port_component = GetUniverse().get<components::infrastructure::SpacePort>(space_port);
        for (auto& [target, delivery_queue] : port_component.deliveries) {
            // Let's ignore the moon for now
            if (target != GetUniverse().planets["moon"]) {
                continue;
            }
            // Launch a ship with the resources and stuff
            while (!delivery_queue.empty()) {
                components::infrastructure::TransportedGood element = delivery_queue.back();
                // Spawn new ship and send it to a parking orbit
                // Get the reference body
                auto& body = GetUniverse().get<components::bodies::Body>(port_component.reference_body);
                // Let's set a parking orbit at 8% of the planet's radius
                // TODO(EhWhoAmI): It should be a factor of the atmosphere in the future
                const double eccentricity = std::fabs(GetUniverse().random->GetRandomNormal(0, 0.000005));
                const double semi_major_axis =
                    GetUniverse().random->GetRandomNormal(body.radius * 1.08, body.radius * 0.01);
                const double inclination = GetUniverse().random->GetRandomNormal(0, 0.05);
                components::types::Orbit orb(semi_major_axis, eccentricity, inclination, 0, 0.1, 0,
                                             port_component.reference_body);
                entt::entity ship = common::actions::LaunchShip(GetUniverse(), orb);
                GetUniverse().emplace<components::Name>(
                    ship, fmt::format("{} Transport Vehicle", util::GetName(GetUniverse(), element.good)));
                // Then target the target body
                // Let's assume that its the moon for now so that we can see that we can automate things
                // Let's add a zero maneuver like 30 minutes in the future and then we can trigger a plane change operation on that
                // commands::PushManeuver(GetUniverse(), ship, commands::MakeManeuver(glm::dvec3(0., 0., 0.), 30 * components::StarDate::MINUTE), 0);
                commands::TransferToMoon(GetUniverse(), ship, target);
                // Add a resource stockpile to the ship
                delivery_queue.pop_back();
            }
        }
    }
}
}  // namespace cqsp::common::systems
