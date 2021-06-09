/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include "engine/application.h"

namespace conquerspace {
namespace client {
namespace systems {

class SysPlanetInformation {
 public:
    void DisplayPlanet(entt::entity &planet, conquerspace::engine::Application& m_app);
    entt::entity selected_city_entity = entt::null;
    int selected_city_index = 0;
    enum class ViewMode { PLANET_VIEW, CITY_VIEW };
    ViewMode view_mode = ViewMode::PLANET_VIEW;
    bool to_see = false;
 private:
    void CityInformationPanel(entt::entity& planet, conquerspace::engine::Application& m_app);
    void PlanetInformationPanel(entt::entity& planet, conquerspace::engine::Application& m_app);
};
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
