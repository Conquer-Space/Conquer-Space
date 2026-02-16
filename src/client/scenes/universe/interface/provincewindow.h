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

#include <string>

#include "client/systems/sysgui.h"

namespace cqsp::client::systems {
class SysProvinceInformation : public SysUserInterface {
 public:
    explicit SysProvinceInformation(engine::Application& app) : SysUserInterface(app) {}
    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    void ProvinceView();

    void ProvinceIndustryTabs();
    void DemographicsTab();
    void IndustryTab();
    void SpacePortTab();
    void SpacePortOrdersTab(const entt::entity city);
    void SpacePortResourceTab(const entt::entity city);
    void LaunchTab(const entt::entity city);
    void DockedTab(const entt::entity city);
    void SpacePortProjectsTab(const entt::entity city);
    void InfrastructureTab();
    void IndustryListWindow();
    void DisplayWallet(entt::entity entity);
    void ConstructionTab();
    bool HasSpacePort(const entt::entity entity);
    void IndustryListIndustryRow(const entt::entity industry);

    template <typename T>
    void IndustryTabGenericChild(const std::string& tabname, const std::string& industryname, const ImVec2& size);

    entt::entity current_city = entt::null;
    bool changed_city = false;
    entt::entity current_province = entt::null;
    bool market_information_panel = false;

    bool visible = false;
    entt::entity current_market = entt::null;
    entt::entity selected_ship = entt::null;
    bool city_factory_info = false;
    entt::entity selected_recipe = entt::null;
    int construction_amount = 1;

    bool segment_prices = true;

    enum class ViewMode { COUNTRY_VIEW, CITY_VIEW } view_mode = ViewMode::COUNTRY_VIEW;
};

}  // namespace cqsp::client::systems
