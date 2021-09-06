#include "common/systems/actions/shiplaunchaction.h"

#include "common/components/movement.h"
#include "common/components/ships.h"
#include "common/components/bodies.h"

entt::entity cqsp::common::systems::actions::CreateShip(
    cqsp::common::Universe& universe, entt::entity civ, entt::entity orbit,
    entt::entity starsystem) {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspb = cqsp::common::components::bodies;
    entt::entity ship = universe.create();
    universe.emplace<cqsps::Ship>(ship);

    auto &position = universe.emplace<cqspt::Position>(ship);

    // Get planet position
    position = cqspt::toVec2(universe.get<cqspt::Orbit>(orbit));
    universe.get<cqspb::StarSystem>(starsystem).bodies.push_back(ship);
    return ship;
    return entt::entity();
}
