/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>

#include "common/universe.h"
#include "common/components/resource.h"

namespace conquerspace {
namespace client {
namespace systems {
void DrawLedgerTable(const std::string &name, conquerspace::common::components::Universe&, conquerspace::common::components::ResourceLedger& ledger);
}  // namespace systems
}  // namespace client
}  // namespace conquerspace
