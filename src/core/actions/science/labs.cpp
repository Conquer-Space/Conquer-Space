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
#include "core/actions/science/labs.h"

#include "core/components/science.h"

namespace cqsp::core::actions {
Node CreateLab(Universe& universe) {
    Node entity(universe);
    // Create labs
    entity.emplace<components::science::Lab>();
    return entity;
}

void AddScienceResearch(Node& lab, Node& research, double progress) {
    // Add the things
    lab.get<components::science::Lab>().science_contribution[research] += progress;
}
}  // namespace cqsp::core::actions
