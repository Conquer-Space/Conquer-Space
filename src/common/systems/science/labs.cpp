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
#include "common/systems/science/labs.h"

#include "common/components/science.h"

namespace cqsp::common::systems::science {
entt::entity CreateLab(Universe& universe) {
    entt::entity entity = universe.create();
    // Create labs
    universe.emplace<components::science::Lab>(entity);
    return entity;
}

void AddScienceResearch(Universe& universe, entt::entity lab, entt::entity research, double progress) {
    // Add the things
    universe.get<components::science::Lab>(lab).science_contribution[research] += progress;
}
}  // namespace cqsp::common::systems::science
