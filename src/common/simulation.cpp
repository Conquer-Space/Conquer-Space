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
#include "common/components/event.h"
#include "common/components/organizations.h"
#include "common/components/player.h"

conquerspace::common::systems::simulation::Simulation::Simulation(
    conquerspace::common::components::Universe &_universe,
    scripting::ScriptInterface &script_interface)
    : m_universe(_universe), script_runner(_universe, script_interface) {
    // Register functions
    script_interface.set_function("event_player", [&](sol::table event_table) {
        sol::table action_list = event_table["actions"];
        std::string name = action_list[1]["name"];
        auto view = m_universe.view<conquerspace::common::components::Player>();
        for (auto b : view) {
            auto& queue = m_universe.get_or_emplace<event::EventQueue>(b);
            auto event = std::make_shared<event::Event>();
            event->title = event_table["title"];
            event->content = event_table["content"];
            event->image = event_table["image"];
            sol::optional<std::vector<sol::table>> optional = event_table["actions"];

            for (auto& action : *optional) {
                auto event_result = std::make_shared<event::EventResult>();
                event_result->name = action["name"];
                sol::optional<std::string> tooltip = action["tooltip"];
                if (tooltip) {
                    event_result->tooltip = *tooltip;
                }

                event->table = event_table;
                sol::function f = action["action"];
                event_result->action = f;
                event->actions.push_back(event_result);
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


    BEGIN_TIMED_BLOCK(ScriptEngine);
    this->script_runner.ScriptEngine();
    END_TIMED_BLOCK(ScriptEngine);

    BEGIN_TIMED_BLOCK(Resource_Gen);

    // Tick date
    auto resource_generator = m_universe.view<cqspc::ResourceGenerator,
                                                                       cqspc::ResourceStockpile>();

    for (entt::entity entity : resource_generator) {
        // Make resources
        cqspc::ResourceGenerator &generator =
                                        m_universe.get<cqspc::ResourceGenerator>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                        m_universe.get<cqspc::ResourceStockpile>(entity);

        // Create resources
        for (auto iterator = generator.begin(); iterator != generator.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.find(iterator->first)!= resource_stockpile.end()) {
                resource_count = resource_stockpile[iterator->first];
            }
            float productivity = 1;
            if (m_universe.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = m_universe.get<cqspc::FactoryProductivity>(entity).productivity;
            }
            resource_count += iterator->second * productivity;
            // Add to resource stockpile
            resource_stockpile[iterator->first] = resource_count;
        }
    }
    END_TIMED_BLOCK(Resource_Gen);

    BEGIN_TIMED_BLOCK(Production);
    auto view = m_universe.view<cqspc::Production,
                                            cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (entt::entity entity : view) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator = m_universe.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                      m_universe.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = m_universe.get<cqspc::Recipe>(generator.recipe);

        // Create resources
        for (auto iterator = recipe.output.begin(); iterator != recipe.output.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.find(iterator->first) != resource_stockpile.end()) {
                resource_count = resource_stockpile[iterator->first];
            }

            float productivity = 1;
            if (m_universe.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = m_universe.get<cqspc::FactoryProductivity>(entity).productivity;
            }
            resource_count += (iterator->second * productivity);
            // Add to resource stockpile

            resource_stockpile[iterator->first] = resource_count;
        }
    }
    END_TIMED_BLOCK(Production);

    BEGIN_TIMED_BLOCK(Resource_Consume);
    // Consume resources
    auto consume = m_universe.view<cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (entt::entity entity : consume) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator =
                                    m_universe.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                    m_universe.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = m_universe.get<cqspc::Recipe>(generator.recipe);
        // Create resources
        for (auto iterator = recipe.input.begin(); iterator != recipe.input.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.find(iterator->first) != resource_stockpile.end()) {
                resource_count = resource_stockpile[iterator->first];
            }
            float productivity = 1;
            if (m_universe.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = m_universe.get<cqspc::FactoryProductivity>(entity).productivity;
            }

            resource_count -= iterator->second * productivity;

            m_universe.emplace_or_replace<cqspc::Production>(entity);

            // Add to resource stockpile
            resource_stockpile[iterator->first] = resource_count;
        }
    }
    END_TIMED_BLOCK(Resource_Consume);
}
