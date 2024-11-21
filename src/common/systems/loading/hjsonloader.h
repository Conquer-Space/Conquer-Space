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

#include "common/universe.h"

namespace cqsp::common::systems::loading {
class HjsonLoader {
 public:
    explicit HjsonLoader(Universe& universe) : universe(universe) {}
    virtual ~HjsonLoader() = default;
    virtual const Hjson::Value& GetDefaultValues() = 0;
    int LoadHjson(const Hjson::Value& values);
    virtual bool LoadValue(const Hjson::Value& values, entt::entity entity) = 0;
    virtual void PostLoad(const entt::entity& entity) {}
    virtual bool NeedIdentifier() { return true; }

 protected:
    Universe& universe;
};
}  // namespace cqsp::common::systems::loading
