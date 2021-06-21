/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include "client/systems/sysgui.h"
#include "engine/application.h"

namespace conquerspace {
namespace client {
namespace systems {

class SysPlanetInformation : public SysUserInterface {
 public:
    explicit SysPlanetInformation(conquerspace::engine::Application& app) : SysUserInterface(app) {}
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
    entt::entity selected_planet = entt::null;
    void CityInformationPanel();
    void PlanetInformationPanel();
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
