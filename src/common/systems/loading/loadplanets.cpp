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
#include "common/systems/loading/loadplanets.h"

#include <spdlog/spdlog.h>

#include <string>

#include "common/systems/loading/loadutil.h"
#include "common/components/coordinates.h"
#include "common/components/bodies.h"

namespace cqsp::common::systems::loading {
bool PlanetLoader::LoadValue(const Hjson::Value& values, Universe& universe,
                             entt::entity entity) {
    using components::types::UnitType;
    // Load orbit
    std::string identifier = values["identifier"];
    const Hjson::Value& orbit = values["orbit"];
    auto& orbit_comp = universe.emplace<components::types::Orbit>(entity);

    universe.emplace<components::bodies::Planet>(entity);
    universe.emplace<components::bodies::Body>(entity);

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
    orbit_comp.CalculatePeriod();

    return true;
}
}  // namespace cqsp::common::systems::loading
