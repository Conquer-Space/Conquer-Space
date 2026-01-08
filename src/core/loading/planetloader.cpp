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

#include "core/loading/planetloader.h"

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <random>
#include <string>
#include <vector>

#include "core/actions/factoryconstructaction.h"
#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/components/population.h"
#include "core/components/surface.h"
#include "core/loading/loadorbit.h"
#include "core/loading/loadutil.h"
#include "core/util/random/random.h"

namespace cqsp::core::loading {

namespace types = components::types;
namespace bodies = components::bodies;
using bodies::Body;
using types::UnitType;

namespace {
struct ParentTemp {
    std::string parent;
};
}  // namespace

bool PlanetLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Load orbit
    std::string identifier = values["identifier"];
    const Hjson::Value& orbit = values["orbit"];
    auto& orbit_comp = node.emplace<types::Orbit>();
    node.emplace<bodies::Planet>();
    auto& body_comp = node.emplace<Body>();
    node.emplace<components::Market>(universe.GoodCount());
    node.emplace<components::PlanetaryMarket>(universe.GoodCount());
    node.emplace<bodies::OrbitalSystem>();

    node.emplace<bodies::NautralObject>();
    if (values["type"].type() != Hjson::Type::Undefined) {
        if (values["type"].type() != Hjson::Type::String) {
            SPDLOG_INFO("Planet type of {} is in incorrect format", identifier);
            return true;
        }
        if (values["type"].to_string() == "star") {
            node.emplace<bodies::LightEmitter>();
        }
    }

    if (values["texture"].type() != Hjson::Type::Undefined) {
        const Hjson::Value& texture = values["texture"];
        auto& texture_comp = node.emplace<components::bodies::TexturedTerrain>();

        if (texture["terrain"].type() != Hjson::Type::String) {
            SPDLOG_INFO("Terrain texture of {} is in incorrect format", identifier);
        }
        texture_comp.terrain_name = texture["terrain"].to_string();

        if (texture["normal"].type() == Hjson::Type::String) {
            texture_comp.normal_name = texture["normal"].to_string();
        }

        if (texture["roughness"].type() == Hjson::Type::String) {
            texture_comp.roughness_name = texture["roughness"].to_string();
        }

        if (texture["province_texture"].type() == Hjson::Type::String) {
            // We probably need a much more rigorous check, like actually verifying if the files actually exist.
            auto& provinces = node.emplace<components::ProvincedPlanet>();
            provinces.province_texture = texture["province_texture"].to_string();
            if (texture["province_map"].type() == Hjson::Type::String) {
                provinces.province_map = texture["province_map"].to_string();
            }
            if (texture["province_definitions"].type() == Hjson::Type::String) {
                provinces.province_definitions = texture["province_definitions"].to_string();
            }
        }
    }

    body_comp.GM = values["gm"].to_double();

    bool rotation_correct;
    if (values["day_length"].type() != Hjson::Type::Null) {
        body_comp.rotation = ReadUnit(values["day_length"].to_string(), UnitType::Time, &rotation_correct);
        if (!rotation_correct) {
            SPDLOG_WARN("Rotation for {} has incorrect units", identifier);
            body_comp.rotation = 0;
        }
    } else {
        body_comp.rotation = 0;
    }

    if (values["day_offset"].type() != Hjson::Type::Null) {
        bool offset_correct;
        body_comp.rotation_offset = ReadUnit(values["day_offset"].to_string(), UnitType::Angle, &offset_correct);
        if (!offset_correct) {
            SPDLOG_WARN("Axial for {} has incorrect units", identifier);
            body_comp.rotation_offset = 0;
        }
    } else {
        body_comp.rotation_offset = 0.;
    }

    bool axial_correct;
    if (values["axial"].type() != Hjson::Type::Null) {
        body_comp.axial = ReadUnit(values["axial"].to_string(), UnitType::Angle, &axial_correct);
        if (!axial_correct) {
            SPDLOG_WARN("Axial for {} has incorrect units", identifier);
            body_comp.axial = 0;
        }
    } else {
        body_comp.axial = 0;
    }

    bool radius_correct;
    body_comp.radius = ReadUnit(values["radius"].to_string(), UnitType::Distance, &radius_correct);
    if (!radius_correct) {
        SPDLOG_WARN("Issue with radius of {}: {}", identifier, values["radius"].to_string());
        return false;
    }

    if (values["reference"].defined()) {
        auto parent_name = values["reference"];
        node.emplace<ParentTemp>(parent_name);
    } else {
        // It's the sun
        universe.sun = node;
    }

    universe.planets[identifier] = node;
    auto orbit_opt = LoadOrbit(orbit);
    if (orbit_opt) {
        orbit_comp = *orbit_opt;
    } else {
        return false;
    }

    std::vector<std::string> tags;
    if (values["tags"].defined()) {
        // Add the tags
        for (int i = 0; i < values["tags"].size(); i++) {
            tags.push_back(values["tags"][i].to_string());
        }
    }
    return true;
}

void PlanetLoader::PostLoad(const Node& node) {
    // Set the parent
    std ::string identifier = node.get<components::Identifier>().identifier;
    auto& orbit = node.get<types::Orbit>();
    Body& body = node.get<Body>();
    body.mass = bodies::CalculateMass(body.GM);

    if (!node.any_of<ParentTemp>()) {
        return;
    }

    auto& parent_temp = node.get<ParentTemp>();

    if (universe.planets.find(parent_temp.parent) == universe.planets.end()) {
        SPDLOG_INFO("{} parent is not found: {}", identifier, parent_temp.parent);
        node.remove<ParentTemp>();
        return;
    }
    Node parent(universe, universe.planets[parent_temp.parent]);
    SPDLOG_INFO("{}'s parent is {}", identifier, parent_temp.parent);
    orbit.reference_body = parent;
    // Set mu
    orbit.GM = universe.get<Body>(parent).GM;
    body.SOI = bodies::CalculateSOI(body.GM, orbit.GM, orbit.semi_major_axis);
    body.mass = bodies::CalculateMass(body.GM);

    parent.get_or_emplace<bodies::OrbitalSystem>().push_back(node);
    parent.get_or_emplace<bodies::OrbitalSystem>().bodies.push_back(node);
    node.remove<ParentTemp>();
}
}  // namespace cqsp::core::loading
