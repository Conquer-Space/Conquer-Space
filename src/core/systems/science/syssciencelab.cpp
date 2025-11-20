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
#include "core/systems/science/syssciencelab.h"

#include <tracy/Tracy.hpp>

#include "core/components/science.h"

void cqsp::core::systems::SysScienceLab::DoSystem() {
    ZoneScoped;
    auto labs = GetUniverse().nodes<components::science::Lab>();
    // Add to the science
    for (Node lab_node : labs) {
        // Add to the scientific progress of the area, I guess
        auto& lab = lab_node.get<components::science::Lab>();
        // Progress the science, I guess
        // Get the science progress, and add to it, somehow
        auto& science_progress = lab_node.get_or_emplace<components::science::ScientificProgress>();
        // Progress science
        science_progress.science_progress.MultiplyAdd(lab.science_contribution, Interval());

        // If the research is done, then research tech
    }
}
