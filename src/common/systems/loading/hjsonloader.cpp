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
#include "common/systems/loading/hjsonloader.h"

#include "common/systems/loading/loadutil.h"

int cqsp::common::systems::loading::HjsonLoader::LoadHjson(
    const Hjson::Value& values, Universe& universe) {
    int assets = 0;
    for (int i = 0; i < values.size(); i++) {
        Hjson::Value value = values[i];

        entt::entity entity = universe.create();
        if (!LoadInitialValues(universe, entity, value)) {
            universe.destroy(entity);
            continue;
        }

        value = Hjson::Merge(GetDefaultValues(), value);
        if (!LoadValue(value, universe, entity)) {
            universe.destroy(entity);
            continue;
        }
        assets++;
    }
    return assets;
}
