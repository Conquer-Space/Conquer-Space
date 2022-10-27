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
#include "common/systems/economy/sysinfrastructure.h"

#include <tracy/Tracy.hpp>

#include "common/components/area.h"
#include "common/components/infrastructure.h"

void cqsp::common::systems::InfrastructureSim::DoSystem() {
    ZoneScoped;
    namespace cqspc = cqsp::common::components;
    Universe& universe = GetUniverse();
    // Get all cities with industry and infrastruture
    auto view = universe.view<cqspc::IndustrialZone>();
    for (entt::entity entity : view) {
        auto& industry = universe.get<cqspc::IndustrialZone>(entity);
        double power_production = 0;
        double power_consumption = 0;
        for (entt::entity industrial_site : industry.industries) {
            if (universe.any_of<cqspc::infrastructure::PowerPlant>(industrial_site)) {
                power_production += universe.get<cqspc::infrastructure::PowerPlant>(industrial_site).production;
            }
            if (universe.any_of<cqspc::infrastructure::PowerConsumption>(industrial_site)) {
                power_consumption += universe.get<cqspc::infrastructure::PowerConsumption>(industrial_site).max;
            }
        }
        // Now assign infrastrutural information
        universe.emplace_or_replace<cqspc::infrastructure::CityPower>(entity, power_production, power_consumption);

        if (power_production < power_consumption) {
            // Then city has no power. Next time, we'd allow transmitting power, or allowing emergency use power
            // but for now, the city will go under brownout.
            universe.get_or_emplace<cqspc::infrastructure::BrownOut>(entity);
        } else {
            universe.remove<cqspc::infrastructure::BrownOut>(entity);
        }
        auto& infra = GetUniverse().get<cqspc::infrastructure::CityInfrastructure>(entity);
        infra.improvement = 0;
        // Add highway things I guess
        if (GetUniverse().any_of<cqspc::infrastructure::Highway>(entity)) {
            infra.improvement +=
                GetUniverse().get<cqspc::infrastructure::Highway>(entity).extent;
        }
    }
}
