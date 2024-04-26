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
#include "common/systems/economy/sysfinance.h"

#include "common/components/economy.h"

namespace components = cqsp::common::components;
using components::Wallet;
using  cqsp::common::systems::SysWalletReset;

void SysWalletReset::DoSystem() {
    for (entt::entity entity : GetUniverse().view<Wallet>()) {
        GetUniverse().get<Wallet>(entity).Reset();
    }
}
