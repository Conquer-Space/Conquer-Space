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

#include <hjson.h>

#include "common/components/resource.h"
#include "common/systems/loading/hjsonloader.h"
#include "common/universe.h"

namespace cqsp::common::systems::loading {
void LoadGoods(Universe&, Hjson::Value&);
void LoadRecipes(Universe&, Hjson::Value&);
void LoadTerrainData(Universe&, Hjson::Value&);

class GoodLoader : public HjsonLoader {
 public:
    explicit GoodLoader(Universe& universe);
    const Hjson::Value& GetDefaultValues() override { return default_val; }
    bool LoadValue(const Hjson::Value& values, entt::entity entity) override;

 private:
    Hjson::Value default_val;
};

class RecipeLoader : public HjsonLoader {
 public:
    explicit RecipeLoader(Universe& universe);
    const Hjson::Value& GetDefaultValues() override { return default_val; }
    bool LoadValue(const Hjson::Value& values, entt::entity entity) override;

 private:
    Hjson::Value default_val;
};
}  // namespace cqsp::common::systems::loading
