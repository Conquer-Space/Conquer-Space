
#include <hjson.h>

#include <string>

#include "core/loading/hjsonloader.h"
#include "core/universe.h"

namespace cqsp::core::actions {
void ResearchTech(Node& civilization, Node& tech);
void ProcessAction(Node& civilization, const std::string& action);
}  // namespace cqsp::core::actions
