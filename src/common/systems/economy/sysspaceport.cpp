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

#include "common/components/spaceport.h"
#include "common/components/bodies.h"

namespace cqsp::common::systems {
void SysSpacePort::DoSystem() {
    auto& space_ports = GetUniverse().view<components::infrastructure::SpacePort>();
    for (entt::entity space_port : space_ports) {
        auto& port_component = GetUniverse().get<components::infrastructure::SpacePort>(space_port);
        for (auto& [good, delivery_queue] : port_component.deliveries) {
            // Launch a ship with the resources and stuff
            while (!delivery_queue.empty()) {
                components::infrastructure::TransportedGood element = delivery_queue.back();
                // Spawn new ship and send it to a parking orbit
                // Get the reference body
                auto& body = GetUniverse().get<components::bodies::Body>(port_component.reference_body);
                // Let's set a parking orbit at 8% of the planet's radius
                // TODO(EhWhoAmI): It should be a factor of the atmosphere in the future
                components::types::Orbit orb(body.radius * 0.08, 0.,
                    components::types::GetLaunchInclination(city_coord.r_latitude(), azimuth), 0,
                    arg_of_perapsis, 0, reference_body);
                entt::entity ship = common::actions::LaunchShip(GetUniverse(), orb);
                GetUniverse().emplace<ctx::VisibleOrbit>(ship);
                delivery_queue.pop_back();
            }
        }
    }
}
}  // namespace cqsp::common::systems
