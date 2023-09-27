/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "common/components/coordinates.h"

#include "common/components/orbit.h"
#include "common/components/units.h"

namespace cqsp::common::components::types {
double GreatCircleDistance(SurfaceCoordinate& coord1, SurfaceCoordinate& coord2) {
    double delta_lambda = abs(coord2.r_longitude() - coord1.r_longitude());
    double alpha = cos(coord2.r_latitude()) * sin(delta_lambda);
    double beta = cos(coord1.r_latitude()) * sin(coord2.r_latitude()) -
                  cos(coord2.r_latitude()) * sin(coord1.r_latitude()) * cos(delta_lambda);
    return atan2(sqrt(alpha * alpha + beta * beta),
                 (sin(coord1.r_latitude()) * sin(coord2.r_latitude()) +
                  cos(coord1.r_latitude()) * cos(coord2.r_latitude()) * cos(delta_lambda)));
}

glm::vec3 toVec3(const SurfaceCoordinate& coord, const float& radius) {
    // This formula is wrong, it's actually
    // x = sin(latitude) * cos (longitude)
    // y = cos latitude
    // z = sin latitude * sin longitude
    // We should probably change it, but it breaks a lot, so we'll leave it here for now.
    return glm::vec3(cos(coord.r_latitude()) * sin(coord.r_longitude()), sin(coord.r_latitude()),
                     cos(coord.r_latitude()) * cos(coord.r_longitude())) *
           radius;
}

double GetLaunchAzimuth(double latitude, double inclination) {
    // https://www.orbiterwiki.org/wiki/Launch_Azimuth
    return asin(cos(inclination) / cos(latitude));
}

double GetLaunchInclination(double latitude, double azimuth) { return acos(cos(latitude) * sin(azimuth)); }

SurfaceCoordinate ToSurfaceCoordinate(const glm::vec3& vec) {
    double latitude = (asin(vec.y));
    double longitude = (atan2(vec.x, vec.z));
    return SurfaceCoordinate(latitude, longitude, true);
}
}  // namespace cqsp::common::components::types
