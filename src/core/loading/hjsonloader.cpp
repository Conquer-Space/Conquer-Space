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

#include <algorithm>
#include <vector>

#include "core/components/name.h"
#include "core/loading/loadutil.h"
#include "core/util/color.h"

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

void TagLoader::ParseTags(const Hjson::Value& tags, Node& node) const {
    for (int i = 0; i < tags.size(); i++) {
        Apply(tags[i].to_string(), node);
    }
}

void TagLoader::Apply(std::string_view tag, Node& node) const {
    auto it = handles.find(std::string(tag));
    if (it != handles.end()) {
        it->second(node);
    } else {
        SPDLOG_WARN("Unknown tag: {}", tag);
    }
}

uint32_t HjsonLoader::StringHash(std::string_view string) {
    const int p = 31;
    const int m = 0xffffff;
    long long hash_value = 0;
    long long p_pow = 1;
    for (char c : string) {
        hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return static_cast<uint32_t>(hash_value);
}

glm::vec3 HjsonLoader::LoadColor(const Hjson::Value& value, std::string_view identifier) {
    if (value.empty() || (value.type() != Hjson::Type::String && value.type() != Hjson::Type::Vector)) {
        uint32_t value = StringHash(identifier);
        return glm::vec3(static_cast<float>(value & 0xFF) / 255.f, static_cast<float>((value >> 8) & 0xFF) / 255.f,
                         static_cast<float>((value >> 16) & 0xFF) / 255.f);
    } else {
        return LoadColor(value);
    }
}

glm::vec3 HjsonLoader::LoadColor(const Hjson::Value& value) {
    std::tuple<int, int, int> color_value;
    if (value.type() == Hjson::Type::Vector) {
        if (value.size() == 3) {
            color_value = std::make_tuple(static_cast<int>(value[0].to_int64()), static_cast<int>(value[1].to_int64()),
                                          static_cast<int>(value[2].to_int64()));
        }
    } else {
        // Then it's probably a string
        color_value = cqsp::util::HexToRgb(value.to_string());
    }

    return glm::vec3(std::clamp(static_cast<float>(std::get<0>(color_value)) / 255.f, 0.f, 1.f),
                     std::clamp(static_cast<float>(std::get<1>(color_value)) / 255.f, 0.f, 1.f),
                     std::clamp(static_cast<float>(std::get<2>(color_value)) / 255.f, 0.f, 1.f));
}
}  // namespace cqsp::core::loading
