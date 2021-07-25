#include "sysmarketplace.h"

#include "common/systems/sysresourcetransfer.h"
#include "common/components/economy.h"
#include "common/components/resource.h"

void conquerspace::common::systems::SysMarketSeller::DoSystem(components::Universe& universe) {
    using conquerspace::common::components::MarketParticipant;
    using conquerspace::common::components::ResourceStockpile;
    using conquerspace::common::components::Market;
    auto view = universe.view<MarketParticipant, ResourceStockpile>();

    for (auto [ent_id, participant, stockpile] : view.each()) {
        // Sell stuff to the market
        auto &market_place = universe.get<Market>(participant.market);
        auto &market_stockpile  = universe.get<ResourceStockpile>(participant.market);
        // Dump the resources to the market
        market_stockpile += stockpile;
        stockpile.clear();
    }
}

void conquerspace::common::systems::SysMarketBuyer::DoSystem(
    components::Universe &universe) {}
