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
#include "common/systems/names/namegenerator.h"

#include <fmt/format.h>
#include <fmt/args.h>

using cqsp::common::systems::names::NameGenerator;
std::string NameGenerator::Generate(const std::string& rule_name) noexcept {
    if (rule_list.find(rule_name) == rule_list.end()) {
        return "";
    }
    if (random == nullptr) {
        return "";
    }

    std::string rule = rule_list[rule_name];
    // Format rule, I guess
    fmt::dynamic_format_arg_store<fmt::format_context> args;
    for (const auto& syllable : syllables_list) {
        if (syllable.second.size() == 0) {
            args.push_back(fmt::arg(syllable.first.c_str(), std::string()));
            continue;
        }
        int index = random->GetRandomInt(0, syllable.second.size() - 1);
        const std::string& sy = syllable.second[index];
        args.push_back(fmt::arg(syllable.first.c_str(), sy));
    }
    try {
        return fmt::vformat(rule, args);
    } catch (const fmt::format_error& error) {
        return "";
    }
}

void NameGenerator::LoadNameGenerator(const Hjson::Value& value) {
    name = value["name"].to_string();
    Hjson::Value rules_hjson = value["rules"];
    for (auto rule : rules_hjson) {
        rule_list[rule.first] = rule.second.to_string();
    }

    for (auto syllable : value) {
        if (syllable.first == "name" || syllable.first == "rules") {
            continue;
        }
        // Parse syllables
        auto& list = syllables_list[syllable.first];
        list.reserve(syllable.second.size());
        for (int i = 0; i < syllable.second.size(); i++) {
            list.emplace_back(syllable.second[i].to_string());
        }
    }
}
