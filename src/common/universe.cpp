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
#include "common/universe.h"

#include <memory>
#include <utility>

#include "common/util/random/stdrandom.h"
#include "common/util/uuid.h"

namespace cqsp::common {
Universe::Universe() : Universe(util::random_id()) {}

Universe::Universe(std::string uuid) : uuid(std::move(uuid)) {
    random = std::make_unique<util::StdRandom>(42);
    nodeFactory = [this](entt::entity entity) { return Node(*this, entity); };
}
}  // namespace cqsp::common
