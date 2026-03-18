#include "core/scripting/orbitfunctions.h"

#include "core/components/orbit.h"
#include "core/scripting/functionreg.h"

namespace cqsp::core::scripting {

namespace bodies = components::bodies;
namespace types = components::types;

void LoadOrbitFunctions(Universe& universe, sol::state_view& script_engine) {
    CREATE_NAMESPACE(core);

    // Register orbit data
    lua_namespace.new_usertype<types::Orbit>(
        "Orbit", sol::constructors<types::Orbit()>(), "GetPeriapsis", &types::Orbit::GetPeriapsis, "eccentricity",
        &types::Orbit::eccentricity, "semi_major_axis", &types::Orbit::semi_major_axis, "inclination",
        &types::Orbit::inclination, "LAN", &types::Orbit::LAN, "w", &types::Orbit::w, "v", &types::Orbit::v, "GM",
        &types::Orbit::GM, "reference_body", &types::Orbit::reference_body);
    REGISTER_FUNCTION("get_orbit", [&](entt::entity entity) { return universe.get<types::Orbit>(entity); });
}
}  // namespace cqsp::core::scripting
