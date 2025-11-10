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
#pragma once

#include <hjson.h>

#include <map>
#include <string>

#include "common/components/resource.h"
#include "common/components/units.h"
#include "common/universe.h"

namespace cqsp::common::loading {
/// <summary>
/// Returns true if name exists
/// </summary>
/// <returns></returns>
bool LoadName(const Node& node, const Hjson::Value& value);
bool LoadIdentifier(const Node& node, const Hjson::Value& value);
bool LoadDescription(const Node& node, const Hjson::Value& value);
bool LoadTags(const Node& node, const Hjson::Value& value);

/// <summary>
/// Loads all the values that should be on every single data type
/// Returns true if an identifier exists
/// </summary>
bool LoadInitialValues(const Node& node, const Hjson::Value& value);

components::ResourceLedger HjsonToLedger(Universe&, Hjson::Value&);

bool VerifyHjsonValueExists(const Hjson::Value& value, const std::string& name, Hjson::Type type);
/// <summary>
/// For the values that *need* to exist
/// </summary>
/// <returns></returns>
bool VerifyInitialValues(const Hjson::Value& value, const std::map<std::string, Hjson::Type>& map);

/// <summary>
/// Gets the unit from the value from expected unit type.
/// </summary>
/// <param name="value"></param>
/// <param name="unit"></param>
/// <param name="correct">[out] if the unit is of the correct type</param>
/// <returns></returns>
double ReadUnit(std::string_view value, components::types::UnitType unit_type, bool* correct = nullptr);
}  // namespace cqsp::common::loading
