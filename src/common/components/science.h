/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include <entt/entt.hpp>

#include "common/components/resource.h"

namespace cqsp::common::components::science {
struct Field {
    std::vector<entt::entity> parents;   // Should probably changed to a set
    std::vector<entt::entity> adjacent;  // same
};

// A scientific research
struct Science {
    int difficulty;
    std::vector<entt::entity> fields;
};

struct Lab {
    ResourceLedger science_contribution;
};

struct ScientificProgress {
    ResourceLedger science_progress;
};

struct ScienceProject {};

struct ScientificResearch {
    // the int is the number of ticks left
    std::map<entt::entity, int> current_research;

    std::set<entt::entity> potential_research;
};

struct TechnologicalProgress {
    std::set<entt::entity> researched_techs;
    // A recipe that has been unlocked? Idk how this should work
    std::set<entt::entity> researched_recipes;
    std::set<entt::entity> researched_mining;
};

struct Technology {
    std::set<entt::entity> fields;
    std::vector<std::string> actions;

    int difficulty;
};
}  // namespace cqsp::common::components::science
