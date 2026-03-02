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
#include "core/systems/economy/syslaunchvehicleproduction.h"

#include <tracy/Tracy.hpp>

#include "core/components/projects.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
void SysLaunchVehicleProduction::DoSystem() {
    ZoneScoped;
    // List cities with launch pads and then add the cost to the economy above...
    auto view = GetUniverse().view<components::City, components::infrastructure::SpacePort>();
    for (entt::entity entity : view) {
        const auto& city_comp = GetUniverse().get<components::City>(entity);
        auto& space_port_comp = GetUniverse().get<components::infrastructure::SpacePort>(entity);
        // Now we get the parent universe and then compute that as well
        entt::entity province = city_comp.province;
        if (!GetUniverse().valid(province)) {
            continue;
        }
        // We add the different projects together and process
        std::vector<entt::entity> completed_projects;
        for (entt::entity project : space_port_comp.projects) {
            // Then we process the project...
            auto& project_comp = GetUniverse().get<components::Project>(project);
            project_comp.progress += Interval();
            if (project_comp.progress >= project_comp.max_progress) {
                // Progress is done, we should delete it somehow
                completed_projects.push_back(project);
                continue;
            }
            // Otherwise we consume resources
            components::ResourceMap& ledger = GetUniverse().get<components::ResourceMap>(project);
            auto& market = GetUniverse().get<components::Market>(province);
            market.consumption += ledger;
            project_comp.project_last_cost = (ledger * market.price).GetSum();
        }

        for (entt::entity completed_project : completed_projects) {
            // Let's also finish the project
            auto& project_comp = GetUniverse().get<components::Project>(completed_project);
            space_port_comp.stored_launch_vehicles.push_back(project_comp.result);

            space_port_comp.projects.erase(
                std::remove(space_port_comp.projects.begin(), space_port_comp.projects.end(), completed_project),
                space_port_comp.projects.end());
            GetUniverse().destroy(completed_project);
        }
    }
}
}  // namespace cqsp::core::systems
