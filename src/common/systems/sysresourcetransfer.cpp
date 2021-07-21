/*
 * Copyright 2021 Conquer Space
 */
#include "common/systems/sysresourcetransfer.h"

#include "common/components/resource.h"

bool conquerspace::common::systems::resource::TransferResources(entt::registry &registry,
                                                        entt::entity from,
                                                        entt::entity to,
                                                        entt::entity good, double amount) {
    namespace cqspc = conquerspace::common::components;
    // Get resource stockpile
    if (!(registry.all_of<cqspc::ResourceStockpile>(from) &&
                            registry.all_of<cqspc::ResourceStockpile>(to) &&
                            registry.all_of<cqspc::Good>(good))) {
        return false;
    }

    // Get resource stockpile
    auto& from_stockpile = registry.get<cqspc::ResourceStockpile>(from);
    auto& to_stockpile = registry.get<cqspc::ResourceStockpile>(from);
    // Transfer resources
    if (from_stockpile.HasGood(good)) {
        // Then we can transfer
        if (from_stockpile[good] >= amount) {
            from_stockpile[good] -= amount;
            to_stockpile[good] += amount;
            return true;
        }
    }
    return false;
}
