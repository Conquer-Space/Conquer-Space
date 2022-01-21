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
#pragma once

#include <hjson.h>

#include <map>
#include <vector>
#include <string>

#include "common/util/random/random.h"

namespace cqsp::common::systems::names {
/// <summary>
/// In the future, I would like to have a more adaptable naming scheme capable of naming cities and other things
/// after people and the like, but for now a simple name generator would do.
/// </summary>
class NameGenerator {
 public:
    std::string Generate(const std::string& rule_name) noexcept;
    void LoadNameGenerator(const Hjson::Value& value);

    void SetRandom(util::IRandom* rand) {
        random = rand;
    }
 private:
    std::map<std::string, std::vector<std::string>> syllables_list;
    std::map<std::string, std::string> rule_list;
    std::string name;

    util::IRandom* random;
};
}  // namespace cqsp::common::systems::names
