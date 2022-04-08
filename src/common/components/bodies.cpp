#include "bodies.h"

cqsp::common::components::bodies::Body::Body() {}

cqsp::common::components::bodies::Body::Body(entt::entity _star_system,
                                             types::kilometer _radius)
    : star_system(_star_system), radius(_radius) {}
