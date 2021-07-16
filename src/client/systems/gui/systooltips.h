/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include "common/universe.h"

namespace conquerspace {
namespace client {
namespace systems {
namespace gui {
void EntityTooltip(entt::entity, conquerspace::common::components::Universe&);
}
}
}
}
