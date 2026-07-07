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
#include "core/loading/powerplantloader.h"

#include "core/components/infrastructure.h"
#include "core/loading/loadutil.h"
#include "core/components/units.h"
#include "core/components/name.h"

namespace cqsp::core::loading {
using components::types::UnitType;

PowerPlantLoader::PowerPlantLoader(Universe& universe) : HjsonLoader(universe) {}

bool PowerPlantLoader::LoadValue(const Hjson::Value& values, Node& node) {
    // Then load power plants
    bool power_correct;
    double output = ReadUnit(values["output"].to_string(), UnitType::Power, &power_correct);
    auto& recipe = node.emplace<components::infrastructure::PowerPlantRecipe>();
    recipe.maximum_production = output;
    recipe.consumption = HjsonToVector(universe, values["input"]);
    universe.infrastructure["power:" + node.get<components::Identifier>().identifier] = node;
    return true;
}
}  // namespace cqsp::core::loading
