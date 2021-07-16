/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <string>
#include <vector>

#include "common/universe.h"
#include "common/scripting/scripting.h"

namespace conquerspace {
namespace common {
namespace systems {
namespace universegenerator {

class ISysUniverseGenerator {
 public:
    virtual void Generate(conquerspace::common::components::Universe& universe) = 0;
};

class ScriptUniverseGenerator : public ISysUniverseGenerator {
 public:
    explicit ScriptUniverseGenerator(conquerspace::scripting::ScriptInterface& _interface) :
                                                                  script_engine(_interface) {}
    void Generate(conquerspace::common::components::Universe& universe);

 private:
    conquerspace::scripting::ScriptInterface& script_engine;
};

}  // namespace universegenerator
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
