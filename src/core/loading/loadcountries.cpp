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
#include "core/loading/loadcountries.h"

#include "core/components/launchvehicle.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/util/logging.h"

namespace cqsp::core::loading {
bool CountryLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Just make the country
    auto& country = node.emplace<components::Country>();
    const std::string& identifier = node.get<components::Identifier>().identifier;
    universe.countries[identifier] = node;

    // Add the list of liabilities the country has?
    if (!values["wallet"].empty()) {
        auto& wallet = node.emplace<components::Wallet>();
        wallet = values["wallet"];
    }

    if (!values["color"].empty() && values["color"].type() == Hjson::Type::Vector && values["color"].size() == 3) {
        country.color[0] = std::clamp(static_cast<float>(values["color"][0].to_double()) / 255.f, 0.f, 1.f);
        country.color[1] = std::clamp(static_cast<float>(values["color"][1].to_double()) / 255.f, 0.f, 1.f);
        country.color[2] = std::clamp(static_cast<float>(values["color"][2].to_double()) / 255.f, 0.f, 1.f);
    } else {
        // Compute string hash on identifier
        uint32_t value = StringHash(identifier);
        country.color[0] = static_cast<float>(value & 0xFF) / 255.f;
        country.color[1] = static_cast<float>((value >> 8) & 0xFF) / 255.f;
        country.color[2] = static_cast<float>((value >> 16) & 0xFF) / 255.f;
    }

    if (!values["tags"].empty() && values["tags"].type() == Hjson::Type::Vector) {
        auto& tags = values["tags"];
        for (size_t i = 0; i < tags.size(); i++) {
            const std::string& tag = tags[i].to_string();
            if (tag == "space_program") {
                node.emplace<components::SpaceCapability>();
                continue;
            }
        }
    }

    return true;
}

uint32_t CountryLoader::StringHash(const std::string& string) {
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
}  // namespace cqsp::core::loading
