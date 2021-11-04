#include "common/systems/systransport.h"

#include "common/components/bodies.h"
#include "common/components/coordinates.h"

cqsp::common::components::types::kilometer
cqsp::common::systems::CalculatePlanetDistance(cqsp::common::Universe& universe,
                                               entt::entity planet,
                                               entt::entity city1,
                                               entt::entity city2) {
    namespace cqspc = cqsp::common::components;
    // Get planet radius
    auto& body = universe.get<cqspc::bodies::Body>(planet);
    auto& city1_pos = universe.get<cqspc::types::SurfaceCoordinate>(city1);
    auto& city2_pos = universe.get<cqspc::types::SurfaceCoordinate>(city2);
    return 0.0;
}
