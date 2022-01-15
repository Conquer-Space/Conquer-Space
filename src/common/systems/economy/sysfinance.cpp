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
#include "common/systems/economy/sysfinance.h"

#include "common/components/economy.h"

namespace cqsp::common::systems {
void SysWalletReset::DoSystem() {
    namespace cqspc = cqsp::common::components;
    auto view = GetUniverse().view<cqspc::Wallet>();
    for (entt::entity entity : view) {
        GetUniverse().get<cqspc::Wallet>(entity).Reset();
    }
}
}  // namespace cqsp::common::systems
