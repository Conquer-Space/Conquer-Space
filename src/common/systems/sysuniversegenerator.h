/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <string>
#include <vector>

#include <sol/sol.hpp>

#include "common/universe.h"

namespace conquerspace {
namespace common {
namespace systems {
namespace universegenerator {

class ISysUniverseGenerator {
 public:
    virtual void Generate(conquerspace::common::components::Universe& universe) = 0;
};

class IScriptUniverseGenerator : public ISysUniverseGenerator {
 public:
    IScriptUniverseGenerator() {
        // Load the scripts
        // Initialize lua environment
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);
    }
    void Generate(conquerspace::common::components::Universe& universe);

    void AddUtility(const std::string&);
    void SetCivGen(const std::string&);
    void SetGalaxyGenerator(const std::string&);
    void SetCivInitializer(const std::string&);
 private:
    sol::state lua;
    // Lua scripts
    std::vector<sol::load_result> utility;

    // Generates civilizations
    sol::load_result civ_gen;
    sol::load_result galaxy_generator;

    // Initializes planets of civilizations
    sol::load_result civ_initializer;
};

}  // namespace universegenerator
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
