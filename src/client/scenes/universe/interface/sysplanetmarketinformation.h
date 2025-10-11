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

#include "client/systems/sysgui.h"

namespace cqsp::client::systems {
class SysPlanetMarketInformation : public SysUserInterface {
 public:
    explicit SysPlanetMarketInformation(engine::Application& app) : SysUserInterface(app) {
      search_text.fill(0);
      per_good_details_search_text.fill(0);
   }
    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    void LocalMarketInformation();
    void InterplanetaryTradeInformation();
    void InterplanetaryTradeRightPanel();
    void InterplanetaryTradeAmounts();
    void PerGoodDetails();
    void PerGoodDetailsRightPanel();

    bool to_see = true;
    entt::entity selected_planet = entt::null;
    entt::entity selected_good = entt::null;
    entt::entity per_good_details_selected = entt::null;
    std::array<char, 255> search_text;
    std::array<char, 255> per_good_details_search_text;
};
}  // namespace cqsp::client::systems
