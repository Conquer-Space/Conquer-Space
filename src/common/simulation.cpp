/*
* Copyright 2021 Conquer Space
*/
#include "common/simulation.h"

#include <spdlog/spdlog.h>

#include <vector>
#include <memory>
#include <string>

#include "common/components/area.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/profiler.h"
#include "common/systems/sysresourcegen.h"
#include "common/systems/economy/sysmarketplace.h"

#include "common/components/event.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/ships.h"
#include "common/components/movement.h"
#include "common/components/units.h"

conquerspace::common::systems::simulation::Simulation::Simulation(
    conquerspace::common::components::Universe &_universe,
    scripting::ScriptInterface &script_interface)
    : m_universe(_universe), script_runner(_universe, script_interface) {
    AddSystem<conquerspace::common::systems::SysResourceGen>();
    AddSystem<conquerspace::common::systems::SysFactoryResourceProduction>();
    // Sell the resources they produce
    AddSystem<conquerspace::common::systems::SysMarketSeller>();
    // Buy resources
    // Request demand, and buy things
    AddSystem<conquerspace::common::systems::SysMarketBuyer>();
    AddSystem<conquerspace::common::systems::SysFactoryResourceConsumption>();
    
    // Register functions
    script_interface.set_function("event_player", [&](sol::table event_table) {
        auto view = m_universe.view<conquerspace::common::components::Player>();
        for (auto b : view) {
            auto& queue = m_universe.get_or_emplace<event::EventQueue>(b);
            auto event = std::make_shared<event::Event>();
            event->title = event_table["title"];
            SPDLOG_INFO("Parsing event \"{}\"", event->title);
            event->content = event_table["content"];
            event->image = event_table["image"];
            sol::optional<std::vector<sol::table>> optional = event_table["actions"];
            if (optional) {
                for (auto &action : *optional) {
                    if (action == sol::nil) {
                        continue;
                    }
                    auto event_result = std::make_shared<event::EventResult>();
                    event_result->name = action["name"];
                    sol::optional<std::string> tooltip = action["tooltip"];
                    if (tooltip) {
                        event_result->tooltip = *tooltip;
                    }

                    event->table = event_table;
                    sol::optional<sol::function> f = action["action"];
                    event_result->has_event = f.has_value();
                    if (f) {
                        event_result->action = *f;
                    }
                    event->actions.push_back(event_result);
                }
            }
            queue.events.push_back(event);
        }
    });
}

void conquerspace::common::systems::simulation::Simulation::tick() {
    m_universe.DisableTick();
    m_universe.date.IncrementDate();
    // Get previous tick spacing
    namespace cqspc = conquerspace::common::components;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;


    BEGIN_TIMED_BLOCK(ScriptEngine);
    this->script_runner.ScriptEngine();
    END_TIMED_BLOCK(ScriptEngine);

    for (auto& sys : system_list) {
        if (m_universe.date.GetDate() % sys->Interval() == 0) {
            sys->DoSystem(m_universe);
        }
    }

    BEGIN_TIMED_BLOCK(Movement);
    auto bodies = m_universe.view<cqspt::Orbit>();
    for (entt::entity body : bodies) {
        auto &orb = cqspt::updateOrbit(m_universe.get<cqspt::Orbit>(body));
        cqspt::updatePos(m_universe.get<cqspt::Kinematics>(body), orb);
    }
    auto bodies2 =
        m_universe.view<cqspt::MoveTarget>(entt::exclude<cqspt::Orbit>);

    for (entt::entity body : bodies2) {
        cqspt::Kinematics & bodykin = m_universe.get<cqspt::Kinematics>(body);
        cqspt::Kinematics & targetkin = m_universe.get<cqspt::Kinematics>(
            m_universe.get<cqspt::MoveTarget>(body).targetent);
        
        SPDLOG_INFO(bodykin.postion.x);
        SPDLOG_INFO(targetkin.postion.x);
        glm::vec3 path = targetkin.postion - bodykin.postion;
        SPDLOG_INFO("Move Target");
        SPDLOG_INFO(distance(targetkin.postion, bodykin.postion));
        if (glm::distance(targetkin.postion, bodykin.postion) <
            bodykin.topspeed) {
            bodykin.postion = targetkin.postion;
            bodykin.velocity = glm::vec3(0, 0, 0);
        } else {
            bodykin.velocity = bodykin.topspeed * glm::normalize(path);
        }
        cqspt::updatePos(m_universe.get<cqspt::Kinematics>(body));
    }
    END_TIMED_BLOCK(Movement);
}
