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
#include "common/loading/loadgoods.h"

#include <spdlog/spdlog.h>

#include <string>
#include <tuple>

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/market.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/loading/loadutil.h"

namespace cqsp::common::loading {
namespace types = components::types;

GoodLoader::GoodLoader(Universe& universe) : HjsonLoader(universe) {
    default_val["price"] = 1.f;
    default_val["tags"] = Hjson::Type::Vector;
}

bool GoodLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    universe.emplace<components::Good>(entity);

    std::string identifier = values["identifier"].to_string();
    if (values["mass"].defined() && values["volume"].defined()) {
        // Then it's matter and physical
        auto& matter = universe.emplace<components::Matter>(entity);
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
        universe.emplace<components::Energy>(entity, t);
    }
    if (values["consumption"].defined()) {
        const Hjson::Value& consumption = values["consumption"];
        double autonomous_consumption = consumption["autonomous_consumption"].to_double();
        double marginal_propensity = consumption["marginal_propensity"].to_double();

        components::ConsumerGood& cg = universe.get_or_emplace<components::ConsumerGood>(entity);
        // We should set the consumption to be consumption over a year
        // So it should be
        cg.autonomous_consumption = autonomous_consumption / static_cast<double>(components::StarDate::YEAR);
        cg.marginal_propensity = marginal_propensity / static_cast<double>(components::StarDate::YEAR);
        SPDLOG_INFO("Creating consumer good {} with autonomous consumption {} and marginal propensity {}", identifier, cg.autonomous_consumption,
                    cg.marginal_propensity);
        universe.consumergoods.push_back(entity);
    }

    for (int i = 0; i < values["tags"].size(); i++) {
        if (values["tags"][i] == "mineral") {
            universe.get_or_emplace<components::Mineral>(entity);
        } else if (values["tags"][i] == "captialgood") {
            universe.get_or_emplace<components::CapitalGood>(entity);
        }
    }

    universe.emplace<components::Price>(entity, values["price"].to_double());

    if (values["unit"].type() == Hjson::Type::String) {
        universe.emplace<components::Unit>(entity, values["unit"].to_string());
    }

    // Basically if it fails at any point, we'll remove the component
    universe.goods[identifier] = entity;
    return true;
}
}  // namespace cqsp::common::loading
