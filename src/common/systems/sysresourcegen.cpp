#include "common/systems/sysresourcegen.h"

#include "common/components/area.h"
#include "common/components/resource.h"

void conquerspace::common::systems::SysResourceGen::DoSystem(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;

    // Tick date
    auto resource_generator = universe.view<cqspc::ResourceGenerator, cqspc::ResourceStockpile>();

    for (entt::entity entity : resource_generator) {
        // Make resources
        cqspc::ResourceGenerator &generator = universe.get<cqspc::ResourceGenerator>(entity);
        cqspc::ResourceStockpile &resource_stockpile = universe.get<cqspc::ResourceStockpile>(entity);

        // Create resources
        for (auto iterator = generator.begin(); iterator != generator.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.find(iterator->first)!= resource_stockpile.end()) {
                resource_count = resource_stockpile[iterator->first];
            }
            float productivity = 1;
            if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
            }
            resource_count += iterator->second * productivity;
            // Add to resource stockpile
            resource_stockpile[iterator->first] = resource_count;
        }
    }
}

void conquerspace::common::systems::SysFactoryResourceProduction::DoSystem(components::Universe &universe) {
    namespace cqspc = conquerspace::common::components;
    auto view = universe.view<cqspc::Production, cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (entt::entity entity : view) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator = universe.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                      universe.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = universe.get<cqspc::Recipe>(generator.recipe);

        // Create resources
        for (auto iterator = recipe.output.begin(); iterator != recipe.output.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.find(iterator->first) != resource_stockpile.end()) {
                resource_count = resource_stockpile[iterator->first];
            }

            float productivity = 1;
            if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
            }
            resource_count += (iterator->second * productivity);
            // Add to resource stockpile

            resource_stockpile[iterator->first] = resource_count;
        }
    }
}

void conquerspace::common::systems::SysFactoryResourceConsumption::DoSystem(components::Universe &universe) {
    namespace cqspc = conquerspace::common::components;

    // Consume resources
    auto consume = universe.view<cqspc::ResourceConverter, cqspc::ResourceStockpile>();
    for (entt::entity entity : consume) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator = universe.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                    universe.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = universe.get<cqspc::Recipe>(generator.recipe);
        // Create resources
        for (auto iterator = recipe.input.begin(); iterator != recipe.input.end(); iterator++) {
            double resource_count = 0;
            if (resource_stockpile.find(iterator->first) != resource_stockpile.end()) {
                resource_count = resource_stockpile[iterator->first];
            }
            float productivity = 1;
            if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
                productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
            }

            resource_count -= iterator->second * productivity;

            universe.emplace_or_replace<cqspc::Production>(entity);

            // Add to resource stockpile
            resource_stockpile[iterator->first] = resource_count;
        }
    }
}
