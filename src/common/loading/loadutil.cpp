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
#include "common/loading/loadutil.h"

#include <spdlog/spdlog.h>

#include <map>
#include <string>

#include "common/components/name.h"
#include "common/components/tags.h"

namespace cqsp::common::loading {
namespace types = components::types;
using types::UnitType;

bool LoadName(const Node& node, const Hjson::Value& value) {
    if (value["name"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["name"].to_string();
    auto& name = node.emplace<components::Name>();
    name.name = identifier;
    return true;
}

bool LoadIdentifier(const Node& node, const Hjson::Value& value) {
    if (value["identifier"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["identifier"].to_string();
    auto& identifier_comp = node.emplace<components::Identifier>();
    identifier_comp.identifier = identifier;
    return true;
}

bool LoadDescription(const Node& node, const Hjson::Value& value) {
    if (value["description"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["description"].to_string();
    auto& identifier_comp = node.emplace<components::Description>();
    identifier_comp.description = identifier;
    return true;
}

bool LoadTags(const Node& node, const Hjson::Value& value) {
    if (value["tags"].type() != Hjson::Type::Vector) {
        return false;
    }
    std::vector<std::string> tags;
    for (int i = 0; i < value["tags"].size(); i++) {
        Hjson::Value tag_value = value["tags"][i];
        if (tag_value.type() != Hjson::Type::String) {
            // We should probably fail
            return false;
        }
        tags.push_back(tag_value.to_string());
    }
    if (!tags.empty()) {
        auto& tag_component = node.emplace_or_replace<components::Tags>();
        tag_component.tags = std::move(tags);
    }
    return true;
}

bool LoadInitialValues(const Node& node, const Hjson::Value& value) {
    LoadName(node, value);
    LoadDescription(node, value);
    LoadTags(node, value);
    return LoadIdentifier(node, value);
}

components::ResourceLedger HjsonToLedger(Universe& universe, Hjson::Value& hjson) {
    components::ResourceLedger stockpile;
    for (auto& input_good : hjson) {
        if (!universe.goods.contains(input_good.first)) {
            // Ideally we'd like to fail out of here but let's just fail silently here for now
            SPDLOG_ERROR("Non-existent good {}, skipping!", input_good.first);
            continue;
        }
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
    } catch (const std::exception&) {
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
}  // namespace cqsp::common::loading
