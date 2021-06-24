/*
* Copyright 2021 Conquer Space
*/
#include "common/simulation.h"

#include <spdlog/spdlog.h>

#include "common/components/area.h"
#include "common/components/name.h"
#include "common/components/resource.h"
#include "common/util/profiler.h"

void conquerspace::common::systems::simulation::Simulation::tick() {
    m_universe.DisableTick();
    // Get previous tick spacing
    namespace cqspc = conquerspace::common::components;

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

    m_universe.date.IncrementDate();
}
