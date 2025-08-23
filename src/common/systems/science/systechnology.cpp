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
#include "common/systems/science/systechnology.h"

#include <vector>

#include <tracy/Tracy.hpp>

#include "common/components/science.h"
#include "common/loading/technology.h"

namespace cqsp::common::systems {

namespace science = components::science;

void SysTechProgress::DoSystem() {
    ZoneScoped;
    auto field = GetUniverse().view<science::ScientificResearch>();

    for (entt::entity entity : field) {
        auto& research = GetUniverse().get<science::ScientificResearch>(entity);
        std::vector<entt::entity> completed_techs;
        for (auto& res : research.current_research) {
            res.second += Interval();
            // Get the research amount
            auto& tech = GetUniverse().get<science::Technology>(res.first);
            if (res.second > tech.difficulty) {
                // Add the researched
                completed_techs.push_back(res.first);
            }
        }
        for (entt::entity r : completed_techs) {
            actions::ResearchTech(GetUniverse(), entity, r);
            research.current_research.erase(r);
        }
    }
}
}  // namespace cqsp::common::systems