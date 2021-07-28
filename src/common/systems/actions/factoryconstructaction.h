#pragma once

#include <entt/entt.hpp>

#include "common/universe.h"

namespace conquerspace {
namespace common {
namespace systems {
namespace actions {
entt::entity ConstructFactory(conquerspace::common::components::Universe& universe,
                      entt::entity city, entt::entity recipe, int productivity);
}
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
