/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>
#include <math.h>
#include "common/components/units.h"
#include "spdlog/spdlog.h"

namespace conquerspace {
namespace common {
namespace components {
namespace types {

/**
 * Orbit of a body
 */
struct Orbit {
    double gravitationalparameter;
    degree theta;
    astronomical_unit semiMajorAxis;
    double eccentricity;
    double angularvelocity;
    degree argument;
    years period;


    // So we can prepare for moons and stuff
    entt::entity referenceBody = entt::null;



    Orbit() = default;
    Orbit(types::degree _trueAnomaly, types::astronomical_unit _semiMajorAxis,
          double _eccentricity, types::degree _argument, double _gravparam)
        : theta(_trueAnomaly),
          semiMajorAxis(_semiMajorAxis),
          eccentricity(_eccentricity),
          argument(_argument),
          gravitationalparameter(_gravparam){}//should be 40
};

inline void updateOrbit(Orbit& orb) { orb.theta += orb.angularvelocity; }

inline void findPeriod(Orbit& orb) { 
    orb.period = TWOPI * std::sqrt(std::pow(orb.semiMajorAxis, 3) /
                                    orb.gravitationalparameter);
    orb.angularvelocity = 360.0 * (1.0 / (orb.period * 365));
}

struct Vec2 {
    astronomical_unit x;
    types::astronomical_unit y;

    Vec2() = default;
    Vec2(types::astronomical_unit _x, types::astronomical_unit _y)
        : x(_x), y(_y) {}
};

struct PolarCoordinate {
    astronomical_unit r;
    degree theta;

    PolarCoordinate() = default;
    PolarCoordinate(types::astronomical_unit _r, types::degree _theta)
        : r(_r), theta(_theta) {}
};

inline types::radian toRadian(types::degree theta) {
    return theta * (conquerspace::common::components::types::PI / 180);
}

inline types::degree toDegree(types::radian theta) {
    return theta * (180 / conquerspace::common::components::types::PI);
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
