#include "markethelpers.h"

#include "common/components/economy.h"
void cqsp::common::systems::economy::AddParticipant(
                cqsp::common::Universe& universe, entt::entity market_entity,
                entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    auto& market = universe.get<cqspc::Market>(market_entity);
    market.participants.insert(entity);
    universe.emplace<cqspc::MarketParticipant>(entity, market_entity);
}
