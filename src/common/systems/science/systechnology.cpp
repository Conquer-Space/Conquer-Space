/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "common/systems/science/technology.h"

using cqsp::common::components::science::Technology;
using cqsp::common::components::science::ScientificResearch;
using cqsp::common::systems::SysTechProgress;
using entt::entity;

void SysTechProgress::DoSystem() {
    ZoneScoped;
    for (entity presearch : GetUniverse().view<ScientificResearch>()) {
        auto& research = GetUniverse().get<ScientificResearch>(presearch);
        std::vector<entity> completed_techs;
        for (auto& res : research.current_research) {
            res.second += Interval();
            // Get the research amount
            auto& tech = GetUniverse().get<Technology>(res.first);
            if (res.second > tech.difficulty) {
                // Add the researched
                completed_techs.push_back(res.first);
            }
        }
        for (entity r : completed_techs) {
            science::ResearchTech(GetUniverse(), presearch, r);
            research.current_research.erase(r);
        }
    }
}
