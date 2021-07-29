#include "markethelpers.h"

#include "common/components/economy.h"
void conquerspace::common::systems::economy::AddParticipant(
                conquerspace::common::components::Universe& universe, entt::entity market_entity,
                entt::entity entity) {
    namespace cqspc = conquerspace::common::components;
    auto& market = universe.get<cqspc::Market>(market_entity);
    market.participants.insert(entity);
    universe.emplace<cqspc::MarketParticipant>(entity, market_entity);
}
