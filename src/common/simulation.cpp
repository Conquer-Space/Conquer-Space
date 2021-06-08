/*
* Copyright 2021 Conquer Space
*/
#include "common/simulation.h"

#include <spdlog/spdlog.h>

#include "common/components/area.h"
#include "common/components/name.h"
#include "common/components/resource.h"

void conquerspace::systems::simulation::Simulation::tick() {
    // Get previous tick spacing
    namespace cqspc = conquerspace::components;
    // Tick date
    auto resource_generator = m_universe.registry.view<cqspc::ResourceGenerator,
                                                                        cqspc::ResourceStockpile>();

    for (entt::entity entity : resource_generator) {
        // Make resources
        cqspc::ResourceGenerator &generator =
                                        m_universe.registry.get<cqspc::ResourceGenerator>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                        m_universe.registry.get<cqspc::ResourceStockpile>(entity);

        // Create resources
        for (auto iterator = generator.output.begin();
                                                iterator != generator.output.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.stored.find(iterator->first)
                                                            != resource_stockpile.stored.end()) {
                resource_count = resource_stockpile.stored[iterator->first];
            }
            float productivity = 1;
            if (m_universe.registry.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = m_universe.registry.get<cqspc::FactoryProductivity>(entity)
                    .productivity;
            }
            resource_count += iterator->second * productivity;
            // Add to resource stockpile
            resource_stockpile.stored[iterator->first] = resource_count;
        }
    }

    auto view = m_universe.registry.view<cqspc::Production,
                                            cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (entt::entity entity : view) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator =
                                    m_universe.registry.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                        m_universe.registry.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = m_universe.registry.get<cqspc::Recipe>(generator.recipe);

        // Create resources
        for (auto iterator = recipe.output.begin(); iterator != recipe.output.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.stored.find(iterator->first)
                                                            != resource_stockpile.stored.end()) {
                resource_count = resource_stockpile.stored[iterator->first];
            }

            float productivity = 1;
            if (m_universe.registry.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity =
                        m_universe.registry.get<cqspc::FactoryProductivity>(entity).productivity;
            }
            resource_count += (iterator->second * productivity);
            // Add to resource stockpile

            resource_stockpile.stored[iterator->first] = resource_count;
        }
    }

    // Consume resources
    auto consume = m_universe.registry.view<cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (entt::entity entity : consume) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator =
                                    m_universe.registry.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                    m_universe.registry.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = m_universe.registry.get<cqspc::Recipe>(generator.recipe);
        // Create resources
        for (auto iterator = recipe.input.begin(); iterator != recipe.input.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.stored.find(iterator->first)
                                                            != resource_stockpile.stored.end()) {
                resource_count = resource_stockpile.stored[iterator->first];
            }
            float productivity = 1;
            if (m_universe.registry.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = m_universe.registry.get<cqspc::FactoryProductivity>(entity)
                    .productivity;
            }

            resource_count -= iterator->second * productivity;

            m_universe.registry.emplace_or_replace<cqspc::Production>(entity);

            // Add to resource stockpile
            resource_stockpile.stored[iterator->first] = resource_count;
        }
    }

    m_universe.date.IncrementDate();
}
