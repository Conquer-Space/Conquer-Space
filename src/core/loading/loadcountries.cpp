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
#include "core/components/science.h"
#include "core/util/color.h"
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
    country.color = LoadColor(values["color"], identifier);

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
    node.emplace<components::MissionQueue>();
    node.emplace<components::science::ScientificResearch>();
    node.emplace<components::OrganizationIncome>();
    return true;
}
}  // namespace cqsp::core::loading
