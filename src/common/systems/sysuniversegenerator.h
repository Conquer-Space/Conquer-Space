/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include "engine/application.h"
#include "common/universe.h"

namespace conquerspace {
namespace common {
namespace systems {
namespace universegenerator {

class ISysUniverseGenerator {
    void Generate(conquerspace::common::components::Universe& universe);
};
void SysGenerateUniverse(conquerspace::engine::Application& application);

}  // namespace universegenerator
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
