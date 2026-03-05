#include "groundtrack.h"

namespace cqsp::core::util {
components::types::SurfaceCoordinate GetGroundTrack(const glm::quat& planet_rotation_matrix,
                                                    const glm::vec3& position) {
    glm::vec3 p = glm::normalize(position);
    return components::types::ToSurfaceCoordinate(glm::inverse(planet_rotation_matrix) * p);
}
}  // namespace cqsp::core::util
