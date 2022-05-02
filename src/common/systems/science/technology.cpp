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
#include "common/systems/science/technology.h"

#include "common/components/science.h"
#include "common/systems/loading/loadutil.h"

namespace cqsp::common::systems::science {
void LoadTechnologies(Universe& universe, Hjson::Value& value) {
    // Load the technologies
    for (int i = 0; i < value.size(); i++) {
        Hjson::Value &element = value[i];
        entt::entity entity = universe.create();
        loading::LoadInitialValues(universe, entity, element);
        auto& tech = universe.emplace<components::science::Technology>(entity);
        // Tech data todo
    }
}
}  // namespace cqsp::common::components::science
