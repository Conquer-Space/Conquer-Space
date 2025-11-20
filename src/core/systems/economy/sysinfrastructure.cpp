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
#include "core/systems/economy/sysinfrastructure.h"

#include <tracy/Tracy.hpp>

#include "core/components/area.h"
#include "core/components/infrastructure.h"
namespace cqsp::core::systems {

namespace infrastructure = components::infrastructure;

void InfrastructureSim::DoSystem() {
    ZoneScoped;
    // Get all cities with industry and infrastruture
    for (Node zone_node : GetUniverse().nodes<components::IndustrialZone>()) {
        auto& industry = zone_node.get<components::IndustrialZone>();
        double power_production = 0;
        double power_consumption = 0;
        for (Node industrial_node : zone_node.Convert(industry.industries)) {
            if (industrial_node.any_of<infrastructure::PowerPlant>()) {
                power_production += industrial_node.get<infrastructure::PowerPlant>().production;
            }
            if (industrial_node.any_of<infrastructure::PowerConsumption>()) {
                power_consumption += industrial_node.get<infrastructure::PowerConsumption>().max;
            }
        }
        // Now assign infrastrutural information
        zone_node.emplace_or_replace<infrastructure::CityPower>(power_production, power_consumption);

        if (power_production < power_consumption) {
            // Then city has no power. Next time, we'd allow transmitting power, or allowing emergency use power
            // but for now, the city will go under brownout.
            zone_node.get_or_emplace<infrastructure::BrownOut>();
        } else {
            zone_node.remove<infrastructure::BrownOut>();
        }
        if (zone_node.any_of<infrastructure::CityInfrastructure>()) {
            auto& infra = zone_node.get<infrastructure::CityInfrastructure>();
            infra.improvement = 0;
            // Add highway things I guess
            if (zone_node.any_of<infrastructure::Highway>()) {
                infra.improvement += zone_node.get<infrastructure::Highway>().extent;
            }
        }
    }
}
}  // namespace cqsp::core::systems