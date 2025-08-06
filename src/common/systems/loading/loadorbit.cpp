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
#include "common/systems/loading/loadorbit.h"

#include "common/components/units.h"
#include "common/systems/loading/loadutil.h"

namespace cqsp::common::systems::loading {

namespace types = components::types;
using types::UnitType;
using types::Orbit;

/// @brief Loads an orbit from a hjson object
/// Note: The caller will have to set the center entity.
/// @param values The hjson input values
/// @return Will return a nullopt if the struct is invalid
std::optional<Orbit> LoadOrbit(const Hjson::Value& values) {

    Orbit orbit;
    if (values["semi_major_axis"].type() != Hjson::Type::String && values["semi_major_axis"].to_double() == 0) {
        orbit.semi_major_axis = 0;
        return std::optional<Orbit>(orbit);
    }

    bool sma_correct;
    orbit.semi_major_axis = ReadUnit(values["semi_major_axis"].to_string(), UnitType::Distance, &sma_correct);
    if (!sma_correct) {
        return std::nullopt;
    }

    orbit.eccentricity = values["eccentricity"].to_double();

    bool inc_correct;
    orbit.inclination = ReadUnit(values["inclination"].to_string(), UnitType::Angle, &inc_correct);
    if (!inc_correct) {
        return std::nullopt;
    }

    bool w_correct;
    orbit.w = ReadUnit(values["arg_periapsis"].to_string(), UnitType::Angle, &w_correct);
    if (!w_correct) {
        return std::nullopt;
    }

    bool LAN_correct;
    orbit.LAN = ReadUnit(values["LAN"].to_string(), UnitType::Angle, &LAN_correct);
    if (!LAN_correct) {
        return std::nullopt;
    }

    bool M0_correct;
    orbit.M0 = ReadUnit(values["M0"].to_string(), UnitType::Angle, &M0_correct);
    if (!M0_correct) {
        return std::nullopt;
    }
    return std::optional<Orbit>(orbit);
}
}  // namespace cqsp::common::systems::loading
