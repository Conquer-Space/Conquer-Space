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
#include "common/systems/loading/loadutil.h"

#include <spdlog/spdlog.h>

#include <map>
#include <string>

#include "common/components/name.h"

namespace cqsp::common::systems::loading {

namespace types = components::types;
using types::UnitType;


bool LoadName(Universe& universe, const entt::entity& entity, const Hjson::Value& value) {
    if (value["name"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["name"].to_string();
    auto& name = universe.emplace<components::Name>(entity);
    name.name = identifier;
    return true;
}

bool LoadIdentifier(Universe& universe, const entt::entity& entity, const Hjson::Value& value) {
    if (value["identifier"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["identifier"].to_string();
    auto& identifier_comp = universe.emplace<components::Identifier>(entity);
    identifier_comp.identifier = identifier;
    return true;
}

bool LoadDescription(Universe& universe, const entt::entity& entity, const Hjson::Value& value) {
    if (value["description"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["description"].to_string();
    auto& identifier_comp = universe.emplace<components::Description>(entity);
    identifier_comp.description = identifier;
    return true;
}

bool LoadInitialValues(Universe& universe, const entt::entity& entity, const Hjson::Value& value) {
    LoadName(universe, entity, value);
    LoadDescription(universe, entity, value);
    return LoadIdentifier(universe, entity, value);
}

components::ResourceLedger HjsonToLedger(Universe& universe, Hjson::Value& hjson) {
    components::ResourceLedger stockpile;
    for (auto& input_good : hjson) {
        stockpile[universe.goods[input_good.first]] = input_good.second;
    }
    return stockpile;
}

bool VerifyHjsonValueExists(const Hjson::Value& value, const std::string& name, Hjson::Type type) {
    return value[name].type() == type;
}

bool VerifyInitialValues(const Hjson::Value& value, const std::map<std::string, Hjson::Type>& map) { return false; }

namespace {
std::string trim(const std::string& str, const std::string& whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";  // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

bool is_number(std::string_view s) {
    char* end = nullptr;
    double val = strtod(s.data(), &end);
    return end != s.data() && *end == '\0' && val != HUGE_VAL;
}
}  // namespace

double ReadUnit(std::string_view value, UnitType unit_type, bool* correct) {

    // Find the letters
    if (correct != nullptr) {
        (*correct) = true;
    }

    auto mark_wrong = [&]() {
        if (correct != nullptr) {
            (*correct) = false;
        }
    };

    std::string content(value);
    content = trim(content);

    std::size_t index = content.find_last_of(' ');
    if (index == std::string::npos) {
        for (index = content.size(); index > 0; index--) {
            if (isalpha(content.at(index - 1)) == 0) {
                break;
            }
        }
    }

    // Get the value in front

    // If it ends with a digit and there's a space, then kill it
    std::string value_string(content.substr(0, index));
    std::string unit_string(content.substr(index, content.size()));

    if (is_number(unit_string)) {
        // Complain
        mark_wrong();
        return 0;
    }

    if (value_string.find(' ') != std::string::npos) {
        mark_wrong();
        return 0;
    }
    unit_string = trim(unit_string);

    double read_value = 0.0;
    try {
        read_value = std::stod(value_string);
    } catch (const std::exception& err) {
        mark_wrong();
        return 0.0;
    }

    // The number
    switch (unit_type) {
        case UnitType::Distance:
            // Lots of distances
            if (unit_string == "km" || unit_string.empty()) {
                // remain as it is
            } else if (unit_string == "AU" || unit_string == "au") {
                read_value = types::toKm(read_value);
            } else if (unit_string == "m") {
                read_value /= 1000.f;
            } else {
                // then it's invalid
                mark_wrong();
            }
            break;
        case UnitType::Angle:
            if (unit_string == "rad") {
                // Remain as it is
            } else if (unit_string.empty() || unit_string == "deg") {
                read_value = types::toRadian(read_value);
            } else {
                // then it's invalid
                mark_wrong();
            }
            break;
        case UnitType::Mass:
            if (unit_string == "kg" || unit_string.empty()) {
                // Remain as it is
            } else if (unit_string == "t") {
                read_value *= 1000;
            } else if (unit_string == "g") {
                read_value /= 1000;
            } else {
                mark_wrong();
            }
            break;
        case UnitType::Volume:
            if (unit_string == "m3" || unit_string.empty()) {
                // Remain as it is
            } else {
                // then it's invalid
                mark_wrong();
            }
            break;
        case UnitType::Time:
            if (unit_string == "s") {
                // Leave empty
            } else if (unit_string == "m") {
                read_value *= 60;
            } else if (unit_string == "h") {
                read_value *= (60 * 60);
            } else if (unit_string == "d") {
                read_value *= (60 * 60 * 24);
            } else {
                mark_wrong();
            }
            break;
    }
    return read_value;
}
}  // namespace cqsp::common::systems::loading
