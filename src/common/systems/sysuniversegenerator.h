/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <string>
#include <vector>

#include "common/scripting/scripting.h"
#include "common/universe.h"

namespace cqsp {
namespace common {
namespace systems {
namespace universegenerator {

class ISysUniverseGenerator {
 public:
    virtual void Generate(common::Universe& universe) = 0;
};

class ScriptUniverseGenerator : public ISysUniverseGenerator {
 public:
    explicit ScriptUniverseGenerator(common::scripting::ScriptInterface& _interface) : script_engine(_interface) {}
    void Generate(common::Universe& universe);

 private:
    common::scripting::ScriptInterface& script_engine;
};

}  // namespace universegenerator
}  // namespace systems
}  // namespace common
}  // namespace cqsp
