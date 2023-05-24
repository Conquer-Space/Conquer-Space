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
#include "common/universe.h"

#include <memory>

#include "common/util/random/stdrandom.h"
#include "common/util/uuid.h"

cqsp::common::Universe::Universe() : Universe(util::random_id()) {}

cqsp::common::Universe::Universe(std::string uuid) : uuid(uuid) {
    random = std::make_unique<cqsp::common::util::StdRandom>(42);
}
