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

#include "common/loading/loadplanets.h"

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <random>
#include <string>
#include <vector>

#include "common/actions/factoryconstructaction.h"
#include "common/loading/loadorbit.h"
#include "common/loading/loadutil.h"

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/market.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/population.h"
#include "common/components/surface.h"
#include "common/util/random/random.h"

namespace cqsp::common::loading {

namespace types = components::types;
namespace bodies = components::bodies;
using types::UnitType;
using bodies::Body;

namespace {
struct ParentTemp {
    std::string parent;
};
}  // namespace

bool PlanetLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    // Load orbit
    std::string identifier = values["identifier"];
    const Hjson::Value& orbit = values["orbit"];
    auto& orbit_comp = universe.emplace<types::Orbit>(entity);
    universe.emplace<bodies::Planet>(entity);
    auto& body_comp = universe.emplace<Body>(entity);
    universe.emplace<components::Market>(entity);
    universe.emplace<components::PlanetaryMarket>(entity);

    universe.emplace<bodies::NautralObject>(entity);
    if (values["type"].type() != Hjson::Type::Undefined) {
        if (values["type"].type() != Hjson::Type::String) {
            SPDLOG_INFO("Planet type of {} is in incorrect format", identifier);
            return true;
        }
        if (values["type"].to_string() == "star") {
            universe.emplace<bodies::LightEmitter>(entity);
        }
    }

    if (values["texture"].type() != Hjson::Type::Undefined) {
        const Hjson::Value& texture = values["texture"];
        auto& texture_comp = universe.emplace<components::bodies::TexturedTerrain>(entity);

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
            auto& provinces = universe.emplace<components::ProvincedPlanet>(entity);
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
            SPDLOG_WARN("Rotation for {} incorrect", identifier);
            body_comp.rotation = 0;
        }
    } else {
        body_comp.rotation = 0;
    }

    if (values["day_offset"].type() != Hjson::Type::Null) {
        bool offset_correct;
        body_comp.rotation_offset = ReadUnit(values["day_offset"].to_string(), UnitType::Angle, &offset_correct);
        if (!offset_correct) {
            SPDLOG_WARN("Axial for {} incorrect", identifier);
            body_comp.rotation_offset = 0;
        }
    } else {
        body_comp.rotation_offset = 0.;
    }

    bool axial_correct;
    if (values["axial"].type() != Hjson::Type::Null) {
        body_comp.axial = ReadUnit(values["axial"].to_string(), UnitType::Angle, &axial_correct);
        if (!axial_correct) {
            SPDLOG_WARN("Axial for {} incorrect", identifier);
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
        universe.emplace<ParentTemp>(entity, parent_name);
    } else {
        // It's the sun
        universe.sun = entity;
    }

    universe.planets[identifier] = entity;
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

void PlanetLoader::PostLoad(const entt::entity& entity) {
    // Set the parent
    auto& orbit = universe.get<types::Orbit>(entity);
    auto& body = universe.get<Body>(entity);
    body.mass = bodies::CalculateMass(body.GM);
    if (!universe.any_of<ParentTemp>(entity)) {
        return;
    }
    auto& parent_temp = universe.get<ParentTemp>(entity);

    if (universe.planets.find(parent_temp.parent) == universe.planets.end()) {
        SPDLOG_INFO("{} parent is not found: {}", universe.get<components::Identifier>(entity).identifier,
                    parent_temp.parent);
        universe.remove<ParentTemp>(entity);
        return;
    }
    entt::entity parent = universe.planets[parent_temp.parent];
    SPDLOG_INFO("{}'s parent is {}", universe.get<components::Identifier>(entity).identifier, parent_temp.parent);
    orbit.reference_body = parent;
    // Set mu
    orbit.GM = universe.get<Body>(parent).GM;
    body.SOI = bodies::CalculateSOI(body.GM, orbit.GM, orbit.semi_major_axis);
    body.mass = bodies::CalculateMass(body.GM);
    universe.get_or_emplace<bodies::OrbitalSystem>(parent).push_back(entity);
    universe.remove<ParentTemp>(entity);
}
}  // namespace cqsp::common::loading
