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
#include "common/systems/science/systechnology.h"

#include "common/components/science.h"

void cqsp::common::systems::SysTechProgress::DoSystem() {
    auto field = GetUniverse().view<components::science::ScientificResearch>();

    for (entt::entity entity : field) {
        auto& research = GetUniverse().get<components::science::ScientificResearch>(entity);
        for (auto& res : research.current_research) {
            res.second += Interval();
        }
    }
}
