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

#include "core/loading/hjsonloader.h"
#include "core/universe.h"

namespace cqsp::core::loading {
class GoodLoader : public HjsonLoader {
 public:
    explicit GoodLoader(Universe& universe);
    const Hjson::Value& GetDefaultValues() override { return default_val; }
    bool LoadValue(const Hjson::Value& values, Node& node) override;

 private:
    Hjson::Value default_val;
    size_t index;
};
}  // namespace cqsp::core::loading
