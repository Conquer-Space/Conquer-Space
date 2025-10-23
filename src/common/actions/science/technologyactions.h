
#include <hjson.h>

#include <string>

#include "common/loading/hjsonloader.h"
#include "common/universe.h"

namespace cqsp::common::actions {
void ResearchTech(Universe &universe, entt::entity civilization, entt::entity tech);
void ProcessAction(Universe &universe, entt::entity civilization, const std::string &action);
}  // namespace cqsp::common::actions
