
#include <hjson.h>

#include <string>

#include "common/loading/hjsonloader.h"
#include "common/universe.h"

namespace cqsp::common::actions {
void ResearchTech(Node civilization, Node tech);
void ProcessAction(Node civilization, const std::string &action);
}  // namespace cqsp::common::actions
