/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <spdlog/spdlog.h>
#include <stdlib.h>

#include <random>
#include <string>

#include "common/systems/loading/loadplanets.h"
#include "common/systems/loading/loadutil.h"
#include "common/systems/actions/factoryconstructaction.h"
//#include "common/components/coordinates.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/surface.h"
#include "common/components/area.h"
#include "common/components/infrastructure.h"
#include "common/components/population.h"
#include "common/components/coordinates.h"
#include "common/components/economy.h"
#include "common/util/random/random.h"

namespace cqspt = cqsp::common::components::types;
namespace cqspc = cqsp::common::components;
namespace cqspa = cqsp::common::systems::actions;

namespace cqsp::common::systems::loading {
namespace {
struct ParentTemp {
    std::string parent;
};
}  // namespace

bool PlanetLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    using components::types::UnitType;
    // Load orbit
    std::string identifier = values["identifier"];
    const Hjson::Value& orbit = values["orbit"];
    auto& orbit_comp = universe.emplace<components::types::Orbit>(entity);
    universe.emplace<components::bodies::Planet>(entity);
    auto& body_comp = universe.emplace<components::bodies::Body>(entity);

    universe.emplace<components::bodies::NautralObject>(entity);
    if (values["type"].type() != Hjson::Type::Undefined) {
        if (values["type"].type() != Hjson::Type::String) {
            SPDLOG_INFO("Planet type of {} is in incorrect format", identifier);
            return true;
        }
        if (values["type"].to_string() == "star") {
            universe.emplace<components::bodies::LightEmitter>(entity);
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
    }

    body_comp.GM = values["gm"].to_double();

    bool rotation_correct;
    if (values["rotation"].type() != Hjson::Type::Null) {
        body_comp.rotation = ReadUnit(values["rotation"].to_string(),
                                      UnitType::Time, &rotation_correct);
        if (!rotation_correct) {
            body_comp.rotation = 0;
        }
    } else {
        body_comp.rotation = 0;
    }

     bool axial_correct;
    if (values["axial"].type() != Hjson::Type::Null) {
        body_comp.rotation = ReadUnit(values["axial"].to_string(),
                                      UnitType::Time, &axial_correct);
        if (!axial_correct) {
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
    util::IRandom* random = universe.random.get();
    if (values["habitation"].type() == Hjson::Type::Bool) {
        SPDLOG_INFO("{} is Habitable", identifier);
        auto& habitats = universe.emplace<cqspc::Habitation>(entity);
        //universe.emplace<cqspc::MarketCenter>(entity).market = entity;
        //universe.emplace<cqspc::Market>(entity);
        if (values["settlements"].type() == Hjson::Type::Int64) {
            int cities = values["settlements"].to_int64();
            for (int i = 0; i < cities; i++) {
                entt::entity newpopulation = universe.create();
                universe.emplace<cqspc::PopulationSegment>(newpopulation)
                    .population = values["population"].to_int64() * 1000000;
                entt::entity newcity = universe.create();
                entt::entity commercial = universe.create();

                universe.emplace<cqspc::Employer>(commercial);
                universe.emplace<cqspc::Commercial>(commercial, newcity, 0);
                universe.emplace<cqspc::Industry>(newcity);
                universe.get<cqspc::Industry>(newcity).industries.push_back(
                    commercial);
                universe.emplace<cqspc::Settlement>(newcity)
                    .population.push_back(newpopulation);
                universe.emplace<cqspt::SurfaceCoordinate>(
                    newcity, random->GetRandomInt(-90, 90), random->GetRandomInt(-180, 180));
                universe.emplace<cqspc::Name>(newcity).name =
                    "City " + std::to_string(i);
                cqspc::ResourceLedger goods =
                    universe.emplace<cqspc::ResourceLedger>(newcity);
                /*
                for (entt::entity entity :
                                 universe.view<cqspc::Matter>()) {
                    goods[entity] = 0;
                }
                */
                SPDLOG_INFO("Making city");
                universe.emplace<cqspc::Employee>(newpopulation);
                for (entt::entity entity : universe.view<cqspc::Recipe>()) {
                    entt::entity factory =
                        cqspa::CreateFactory(universe, newcity, entity, 1);
                }


                universe.emplace<cqspc::infrastructure::SpacePort>(newcity);
                habitats.settlements.push_back(newcity);
            }
        }
    }



    if (values["reference"].defined()) {
        auto parent_name = values["reference"];
        universe.emplace<ParentTemp>(entity, parent_name);
    } else {
        // It's the sun
        universe.sun = entity;
    }

    universe.planets[identifier] = entity;

    if (orbit["semi_major_axis"].type() != Hjson::Type::String && orbit["semi_major_axis"].to_double() == 0) {
        SPDLOG_INFO("Semi major axis of {} is zero", identifier);
        return true;
    }

    bool sma_correct;
    orbit_comp.semi_major_axis = ReadUnit(orbit["semi_major_axis"].to_string(), UnitType::Distance, &sma_correct);
    if (!sma_correct) {
        SPDLOG_WARN("Issue with semi major axis of {}: {}", identifier, orbit["semi_major_axis"].to_string());
        return false;
    }

    orbit_comp.eccentricity = orbit["eccentricity"].to_double();

    bool inc_correct;
    orbit_comp.inclination = ReadUnit(orbit["inclination"].to_string(), UnitType::Angle, &inc_correct);
    if (!inc_correct) {
        SPDLOG_WARN("Issue with inclination of {}: {}", identifier, orbit["inclination"].to_string());
        return false;
    }

    bool w_correct;
    orbit_comp.w = ReadUnit(orbit["arg_periapsis"].to_string(), UnitType::Angle, &w_correct);
    if (!w_correct) {
        SPDLOG_WARN("Issue with arg of periapsis of {}: {}", identifier, orbit["arg_periapsis"].to_string());
        return false;
    }

    bool LAN_correct;
    orbit_comp.LAN = ReadUnit(orbit["LAN"].to_string(), UnitType::Angle, &LAN_correct);
    if (!LAN_correct) {
        SPDLOG_WARN("Issue with LAN of {}: {}", identifier, orbit["LAN"].to_string());
        return false;
    }

    bool M0_correct;
    orbit_comp.M0 = ReadUnit(orbit["M0"].to_string(), UnitType::Angle, &M0_correct);
    if (!M0_correct) {
        SPDLOG_WARN("Issue with mean anomaly of {}: {}", identifier, orbit["M0"].to_string());
        return false;
    }
    return true;
}

void PlanetLoader::PostLoad(const entt::entity& entity) {
    // Set the parent
    auto& orbit = universe.get<components::types::Orbit>(entity);
    auto& body =  universe.get<components::bodies::Body>(entity);
    body.mass = components::bodies::CalculateMass(body.GM);
    if (!universe.any_of<ParentTemp>(entity)) {
        return;
    }
    auto& parent_temp = universe.get<ParentTemp>(entity);

    if (universe.planets.find(parent_temp.parent) == universe.planets.end()) {
        SPDLOG_INFO("{} parent is not found: {}",
                universe.get<components::Identifier>(entity).identifier, parent_temp.parent);
        orbit.CalculateVariables();
        universe.remove<ParentTemp>(entity);
        return;
    }
    entt::entity parent = universe.planets[parent_temp.parent];
    SPDLOG_INFO("{}'s parent is {}", universe.get<components::Identifier>(entity).identifier,
                parent_temp.parent);
    orbit.reference_body = parent;
    // Set mu
    orbit.Mu = universe.get<components::bodies::Body>(parent).GM;
    body.SOI = components::bodies::CalculateSOI(body.GM, orbit.Mu, orbit.semi_major_axis);
    body.mass = components::bodies::CalculateMass(body.GM);
    orbit.CalculateVariables();
    universe.get_or_emplace<components::bodies::OrbitalSystem>(parent).push_back(entity);
    universe.remove<ParentTemp>(entity);
}
}  // namespace cqsp::common::systems::loading
