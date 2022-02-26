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
#pragma once

#include <entt/entt.hpp>

#include "client/systems/sysgui.h"
#include "engine/application.h"

namespace cqsp {
namespace client {
namespace systems {

class SysPlanetInformation : public SysUserInterface {
 public:
    explicit SysPlanetInformation(cqsp::engine::Application& app) : SysUserInterface(app) {}
    void DisplayPlanet();

    entt::entity selected_city_entity = entt::null;
    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

    int selected_city_index = 0;
    enum class ViewMode { PLANET_VIEW, CITY_VIEW };
    ViewMode view_mode = ViewMode::PLANET_VIEW;
    bool to_see = false;

 private:
    void CityInformationPanel();
    void PlanetInformationPanel();
    void ResourcesTab();
    void IndustryTab();
    void IndustryTabServicesChild();
    void IndustryTabManufacturingChild();
    void IndustryTabMiningChild();
    void IndustryTabAgricultureChild();
    void DemographicsTab();
    void ConstructionTab();
    void FactoryConstruction();
    void MineConstruction();
    void MineInformationPanel();
    void FactoryInformationPanel();
    void SpacePortTab();
    void InfrastructureTab();
    void MarketInformationTooltipContent();
    void ConstructionConfirmationPanel();

    entt::entity selected_planet = entt::null;
    bool mine_list_panel = false;
    bool factory_list_panel = false;
    bool power_plant_output_panel = false;
    bool market_information_panel = false;
    bool enable_construction_confirmation_panel = false;
    entt::entity power_plant_changing;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
