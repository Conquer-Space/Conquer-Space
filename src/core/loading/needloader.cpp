/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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
#include "core/loading/needloader.h"

#include "core/components/needs.h"
#include <spdlog/spdlog.h>

namespace cqsp::core::loading {
bool NeedLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Then soemthing
    std::string identifier = GetIdentifier(node);
    universe.needs[identifier] = node.entity();
    auto& need = node.emplace<components::Need>();
    need.priority = LoadDouble(values, "priority", 0);
    need.minimum = LoadDouble(values, "minimum", 0);
    std::string default_good = RequiredString(values, "default");
    if (universe.goods.contains(default_good)) {
        need.default_good = universe.good_map[universe.goods[default_good]];
    } else {
        SPDLOG_INFO("{} does not contain default good ({})!", identifier, default_good);
        return false;
    }
    // Now get the goods
    Hjson::Value good_list = LoadMap(values, "goods");
    for (int i = 0; i < good_list.size(); i++) {
        auto& good_value = good_list[i];
        // Then load the good and set the weight
        std::string good_name = LoadString(good_value, "good");
        if (!universe.goods.contains(good_name)) {
            SPDLOG_WARN("{}: good {} does not exist for the definition!", identifier, good_name);
            continue;
        }
        components::GoodEntity good = universe.good_map[universe.goods[good_name]];
        need.fulfillment_goods[good] = LoadDouble(good_value, "weight", 1);
    }

    // Also compute what goods are associated with it
    return true;
}
}  // namespace cqsp::core::loading
