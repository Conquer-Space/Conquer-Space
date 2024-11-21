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
#include "common/systems/loading/hjsonloader.h"

#include <spdlog/spdlog.h>

#include <vector>

#include "common/components/name.h"
#include "common/systems/loading/loadutil.h"

namespace cqsp::common::systems::loading {
int HjsonLoader::LoadHjson(const Hjson::Value& values) {
    int assets = 0;
    std::vector<entt::entity> entity_list;
    for (int i = 0; i < values.size(); i++) {
        Hjson::Value value = values[i];

        entt::entity entity = universe.create();
        if (NeedIdentifier()) {
            if (!LoadInitialValues(universe, entity, value)) {
                SPDLOG_WARN("No identifier");
                universe.destroy(entity);
                continue;
            }
        } else {
            LoadInitialValues(universe, entity, value);
        }

        value = Hjson::Merge(GetDefaultValues(), value);

        // Catch errors
        bool success = false;
        try {
            success = LoadValue(value, entity);
        } catch (Hjson::index_out_of_bounds& ioob) {
            auto& id = universe.get<components::Identifier>(entity).identifier;
            SPDLOG_WARN("Index out of bounds for {}: {}", id, ioob.what());
        } catch (Hjson::type_mismatch& tm) {
            auto& id = universe.get<components::Identifier>(entity).identifier;
            SPDLOG_WARN("Type mismatch for {}: {}", id, tm.what());
        }

        if (!success) {
            universe.destroy(entity);
            continue;
        }
        entity_list.push_back(entity);
        assets++;
    }

    // Load all the assets again to parse?
    for (entt::entity entity : entity_list) {
        PostLoad(entity);
    }

    return assets;
}
}  // namespace cqsp::common::systems::loading
