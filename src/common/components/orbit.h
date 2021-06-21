/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <boost/math/constants/constants.hpp>
#include <entt/entt.hpp>

#include "common/components/units.h"

namespace conquerspace {
namespace common {
namespace components {
namespace bodies {

/**
 * Orbit of a body
 */
struct Orbit {
    types::degree theta;
    types::astronomical_unit semiMajorAxis;
    double eccentricity;
    types::degree argument;

    // So we can prepare for moons and stuff
    entt::entity referenceBody = entt::null;

    Orbit() = default;
    Orbit(types::degree _trueAnomaly, types::astronomical_unit _semiMajorAxis,
          double _eccentricity, types::degree _argument)
        : theta(_trueAnomaly),
          semiMajorAxis(_semiMajorAxis),
          eccentricity(_eccentricity),
          argument(_argument) {}
};

struct Vec2 {
    types::astronomical_unit x;
    types::astronomical_unit y;

    Vec2() = default;
    Vec2(types::astronomical_unit _x, types::astronomical_unit _y)
        : x(_x), y(_y) {}
};

struct PolarCoordinate {
    types::astronomical_unit r;
    types::degree theta;

    PolarCoordinate() = default;
    PolarCoordinate(types::astronomical_unit _r, types::degree _theta)
        : r(_r), theta(_theta) {}
};

inline types::radian toRadian(types::degree theta) {
    return theta * (boost::math::constants::pi<double>() / 180);
}

inline types::degree toDegree(types::radian theta) {
    return theta * (180 / boost::math::constants::pi<double>());
}

inline Vec2 toVec2(const PolarCoordinate& coordinate) {
    return Vec2{static_cast<types::astronomical_unit>(
                    static_cast<double>(coordinate.r) *
                    cos(toRadian(coordinate.theta))),
                static_cast<types::astronomical_unit>(
                    static_cast<double>(coordinate.r) *
                    sin(toRadian(coordinate.theta)))};
}

inline PolarCoordinate toPolarCoordinate(const Orbit& orb) {
    double r = orb.semiMajorAxis * (1 - orb.eccentricity * orb.eccentricity) /
               (1 - orb.eccentricity *
                        cos(toRadian(fmod(orb.theta, 360) + orb.argument)));
    return PolarCoordinate{(types::astronomical_unit)r, fmod(orb.theta, 360)};
}

inline Vec2 toVec2(const Orbit& orb) { return toVec2(toPolarCoordinate(orb)); }

}  // namespace bodies
}  // namespace components
}  // namespace common
}  // namespace conquerspace
