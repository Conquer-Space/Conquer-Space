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

#include "common/systems/loading/hjsonloader.h"

namespace cqsp::common::systems::loading {
/// <summary>
/// This loader has to be loaded after \ref PlanetLoader because it adds the cities to the
/// respectve planets
/// </summary>
class CityLoader : public HjsonLoader {
   public:
    explicit CityLoader(Universe& universe) : HjsonLoader(universe) {}

    const Hjson::Value& GetDefaultValues() override { return default_val; }
    bool LoadValue(const Hjson::Value& values, entt::entity entity) override;
    void PostLoad(const entt::entity& entity) override;

   private:
    Hjson::Value default_val;
};
}  // namespace cqsp::common::systems::loading
