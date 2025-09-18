
#include <entt/entt.hpp>

#include "common/components/orbit.h"

namespace cqsp::common::components {
enum class Command {
    CircularizeAtApoapsis,
    CircularizeAtPeriapsis,
    SetApoapsis,
    SetPeriapsis,
    SetInclination,
    MatchPlanes,
    CoplanarIntercept,             // Just intercepts
    CoplanarInterceptAndTransfer,  // Intercepts and matches orbit
    Impulse,
    LandOnBody,
};

enum class Trigger {
    // When to trigger the command
    OnEnterSOI,
    OnExitSOI,
    OnManeuver,  // When the previous manuever is executed
    OnCrash,
};

struct OrbitTarget {
    types::Orbit orbit;
};

struct OrbitScalar {
    double value;  // Modifies the orbit by some sort of scalar
};

struct OrbitEntityTarget {
    entt::entity target;
};
}  // namespace cqsp::common::components