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
#include "common/systems/movement/sysmovement.h"
#include "common/systems/syseconomy.h"

#include "common/components/event.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/ships.h"
#include "common/components/movement.h"
#include "common/components/units.h"

using conquerspace::common::systems::simulation::Simulation;
using conquerspace::common::components::Universe;
Simulation::Simulation(Universe &_universe,scripting::ScriptInterface &script_interface) :
    m_universe(_universe), script_runner(_universe, script_interface) {
    namespace cqspcs = conquerspace::common::systems;
    AddSystem<cqspcs::SysFactory>();
    AddSystem<cqspcs::SysOrbit>();
    AddSystem<cqspcs::SysPath>();
    
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

    BEGIN_TIMED_BLOCK(Game_Loop);
    BEGIN_TIMED_BLOCK(ScriptEngine);
    this->script_runner.ScriptEngine();
    END_TIMED_BLOCK(ScriptEngine);

    for (auto& sys : system_list) {
        if (m_universe.date.GetDate() % sys->Interval() == 0) {
            sys->DoSystem(m_universe);
        }
    }
    END_TIMED_BLOCK(Game_Loop);
}
