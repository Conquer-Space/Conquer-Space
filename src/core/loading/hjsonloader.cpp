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
#include "core/loading/hjsonloader.h"

#include <spdlog/spdlog.h>

#include <vector>

#include "core/components/name.h"
#include "core/loading/loadutil.h"

namespace cqsp::core::loading {
/**
 * Loads the hjson struct for an entire asset.
 */
int HjsonLoader::LoadHjson(const Hjson::Value& values) {
    int assets = 0;
    std::vector<Node> node_list;
    for (int i = 0; i < values.size(); i++) {
        Hjson::Value value = values[i];

        Node node(universe);
        if (NeedIdentifier()) {
            if (!LoadInitialValues(node, value)) {
                SPDLOG_WARN("No identifier");
                universe.destroy(node);
                continue;
            }
        } else {
            LoadInitialValues(node, value);
        }

        value = Hjson::Merge(GetDefaultValues(), value);

        // Catch errors
        bool success = false;
        try {
            success = LoadValue(value, node);
        } catch (Hjson::index_out_of_bounds& ioob) {
            auto& id = node.get<components::Identifier>().identifier;
            SPDLOG_WARN("Index out of bounds for {}: {}", id, ioob.what());
        } catch (Hjson::type_mismatch& tm) {
            auto& id = node.get<components::Identifier>().identifier;
            SPDLOG_WARN("Type mismatch for {}: {}", id, tm.what());
        }

        if (!success) {
            universe.destroy(node);
            continue;
        }
        node_list.push_back(node);
        assets++;
    }

    // Load all the assets again to parse?
    for (Node node : node_list) {
        PostLoad(node);
    }

    return assets;
}
}  // namespace cqsp::core::loading
