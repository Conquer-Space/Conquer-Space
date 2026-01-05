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
#include "core/loading/loadgoods.h"

#include <spdlog/spdlog.h>

#include <string>
#include <tuple>

#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/resource.h"
#include "core/loading/loadutil.h"

namespace cqsp::core::loading {
namespace types = components::types;

GoodLoader::GoodLoader(Universe& universe) : HjsonLoader(universe) {
    default_val["price"] = 1.f;
    default_val["tags"] = Hjson::Type::Vector;
}

bool GoodLoader::LoadValue(const Hjson::Value& values, Node& node) {
    node.emplace<components::Good>();

    std::string identifier = values["identifier"].to_string();
    if (values["mass"].defined() && values["volume"].defined()) {
        // Then it's matter and physical
        auto& matter = node.emplace<components::Matter>();
        bool mass_correct;
        matter.mass = ReadUnit(values["mass"].to_string(), types::Mass, &mass_correct);
        if (!mass_correct) {
            SPDLOG_WARN("Mass is formatted incorrectly for {}: {}", identifier, values["mass"].to_string());
            return false;
        }

        bool volume_correct;
        matter.volume = ReadUnit(values["volume"].to_string(), types::Volume, &volume_correct);
        if (!volume_correct) {
            SPDLOG_WARN("Volume is formatted incorrectly for {}: {}", identifier, values["volume"].to_string());
            return false;
        }
    }

    if (values["energy"].defined()) {
        double t = values["energy"];
        node.emplace<components::Energy>(t);
    }
    if (values["consumption"].defined()) {
        const Hjson::Value& consumption = values["consumption"];
        double autonomous_consumption = consumption["autonomous_consumption"].to_double();
        double marginal_propensity = consumption["marginal_propensity"].to_double();

        components::ConsumerGood& cg = node.get_or_emplace<components::ConsumerGood>();
        // We should set the consumption to be consumption over a year
        // So it should be
        cg.autonomous_consumption = autonomous_consumption / static_cast<double>(components::StarDate::YEAR);
        cg.marginal_propensity = marginal_propensity / static_cast<double>(components::StarDate::YEAR);
        SPDLOG_INFO("Creating consumer good {} with autonomous consumption {} and marginal propensity {}", identifier,
                    cg.autonomous_consumption, cg.marginal_propensity);
        universe.consumergoods.push_back(node);
    }

    for (int i = 0; i < values["tags"].size(); i++) {
        if (values["tags"][i] == "mineral") {
            node.get_or_emplace<components::Mineral>();
        } else if (values["tags"][i] == "captialgood") {
            node.get_or_emplace<components::CapitalGood>();
        }
    }

    node.emplace<components::Price>(values["price"].to_double());

    if (values["unit"].type() == Hjson::Type::String) {
        node.emplace<components::Unit>(values["unit"].to_string());
    }

    // Basically if it fails at any point, we'll remove the component
    universe.goods[identifier] = node;
    universe.good_vector.push_back(node);
    universe.good_map[node] = index;
    index++;
    return true;
}
}  // namespace cqsp::core::loading
