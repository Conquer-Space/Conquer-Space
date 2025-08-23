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
#include <vector>

#include "common/util/random/random.h"

namespace cqsp::common::actions {
/// <summary>
/// Name generator is pretty straightforward
/// Format:
/// ```
/// {
///     name: [generator name]
///     rules: {
///         [rule name]: [format]
///     }
///     [syllable name]: [
///         [Array of syllables]
///     ]
///     [syllable name]: [
///     ]
///     // more syllables
/// }
/// ```
/// The format string is processed in fmt, so it would look like this:
/// `{[syllable 1 name]}{[syllable 2 name]}`.
///
/// For each syllable, a random syllable will be selected from the array, according to the random generator
/// that you provide, and formatted into the string.
///
/// In the future, I would like to have a more adaptable naming scheme capable of naming cities and other things
/// after people and the events, but for now a simple name generator would do.
/// </summary>
class NameGenerator {
 public:
    std::string Generate(const std::string& rule_name);
    void LoadNameGenerator(const Hjson::Value& value);

    void SetRandom(util::IRandom* rand) { random = rand; }

    const std::string& GetName() { return name; }

 private:
    std::map<std::string, std::vector<std::string>> syllables_list;
    std::map<std::string, std::string> rule_list;
    std::string name;

    util::IRandom* random;
};
}  // namespace cqsp::common::actions
