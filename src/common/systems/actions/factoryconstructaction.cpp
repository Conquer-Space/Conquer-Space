#include "common/systems/actions/factoryconstructaction.h"

#include "common/components/resource.h"
#include "common/components/area.h"

#include <spdlog/spdlog.h>

using conquerspace::common::components::Universe;
entt::entity conquerspace::common::systems::actions::ConstructFactory(Universe& universe, entt::entity city,entt::entity recipe, int productivity) {
    namespace cqspc = conquerspace::common::components;
    // Make the factory
    entt::entity factory = universe.create();
    auto& factory_converter = universe.emplace<cqspc::ResourceConverter>(factory);
    universe.emplace<cqspc::Factory>(factory);

    // Add producivity
    auto& prod = universe.emplace<cqspc::FactoryProductivity>(factory);
    prod.productivity = productivity;

    universe.emplace<cqspc::ResourceStockpile>(factory);

    // Add recipes and stuff
    factory_converter.recipe = recipe;
    universe.get<cqspc::Industry>(city).industries.push_back(factory);
    return factory;
}
