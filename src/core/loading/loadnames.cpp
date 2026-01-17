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
#include "core/loading/loadnames.h"

#include "core/actions/names/namegenerator.h"

namespace cqsp::core::loading {
void LoadNameLists(Universe &universe, const Hjson::Value &value) {
    for (int i = 0; i < value.size(); i++) {
        const Hjson::Value &name_list = value[i];
        actions::NameGenerator name_gen;
        name_gen.SetRandom(universe.random.get());

        name_gen.LoadNameGenerator(name_list);
        universe.name_generators[name_gen.GetName()] = name_gen;
    }
}
}  // namespace cqsp::core::loading
