/* Conquer Space
* Copyright (C) 2021 Conquer Space
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

#include <entt/entt.hpp>
#include <math.h>
#include "common/components/units.h"
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "bodies.h"

namespace cqsp {
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
    glm::vec3 rotation = glm::vec3(0, 1, 0);

    // So we can prepare for moons and stuff
    entt::entity referenceBody = entt::null;

    Orbit() = default;
    Orbit(types::degree _trueAnomaly, types::astronomical_unit _semiMajorAxis,
          double _eccentricity, types::degree _argument, double _gravparam): 
          theta(_trueAnomaly),
          semiMajorAxis(_semiMajorAxis),
          eccentricity(_eccentricity),
          argument(_argument),
          gravitationalparameter(_gravparam){}//should be 40
};

/*
struct Vec2 {
    astronomical_unit x;
    astronomical_unit y;

    Vec2() = default;
    constexpr Vec2(astronomical_unit _x, astronomical_unit _y)
        : x(_x), y(_y) {}

    constexpr Vec2(const Vec2& v) : x(v.x), y(v.y) {}

    template<class T>
    constexpr Vec2(const T& v) : x(v.x), y(v.y) {}

    template<class T>
    constexpr operator T() const {
        return T(this->x, this->y);
    }

    constexpr Vec2& operator+=(const Vec2& v) {
        this->x += v.x;
        this->y += v.y;
        return *this;
    }

    template<class T>
    constexpr Vec2& operator+=(const T& v) {
        this->x += v.x;
        this->y += v.y;
        return *this;
    }

    constexpr Vec2& operator-=(const Vec2& v) {
        this->x -= v.x;
        this->y -= v.y;
        return *this;
    }

    
    template<class T>
    constexpr Vec2& operator-=(const T& v) {
        this->x -= v.x;
        this->y -= v.y;
        return *this;
    }

    template<typename T>
    constexpr Vec2& operator*=(T scalar) {
        this->x *= scalar;
        this->y *= scalar;
        return *this;
    }

    template<typename T>
    constexpr Vec2& operator/=(T scalar) {
        this->x /= scalar;
        this->y /= scalar;
        return *this;
    }

    constexpr Vec2 operator+(const Vec2& c) const {
        return Vec2(this->x + c.x, this->y + c.y);
    }

    constexpr Vec2 operator-(const Vec2& c) const {
        return Vec2(this->x - c.x, this->y - c.y);
    }

    template<typename T>
    constexpr Vec2 operator*(T scalar) const {
        return Vec2(this->x * scalar, this->y * scalar);
    }

    template<typename T, class B>
    friend constexpr Vec2 operator*(T scalar, const B& v) {
        return Vec2(v.x * scalar, v.y * scalar);
    }

    bool operator==(const Vec2& v) {
        return this->x == v.x && this->y == v.y;
    }

    template<typename T>
    bool operator==(const T& v) {
        return this->x == v.x && this->y == v.y;
    }

    template<class T>
    operator Vec2() {
        return T(this->x, this->y);
    }

    template<typename T>
    constexpr Vec2 operator/(T scalar) const {
        return Vec2(this->x / scalar, this->y / scalar);
    }

    double distance(const Vec2& v) { return distance(*this, v); }

    double length() {
        return length(*this);
    }

    Vec2 normalize() {
        return normalize(*this);
    }

    static double distance(const Vec2& a, const Vec2& b) {
        return length((a - b));
    }

    static double length(const Vec2& v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    static Vec2 normalize(const Vec2& v) {
        return v/length(v);
    }

    double angle() {
        return angle(*this);
    }

    static double angle(const Vec2& v) {
        return atan2(v.y, v.x);
    }

    template<class T>
    static double angle(const T& v) {
        return atan2(v.y, v.x);
    }
};


*/
struct Kinematics {
    glm::vec3 postion = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    float topspeed = 0.1;
    // glm::vec3 target;
    // entt::entity targetent;
};




template<typename T>
struct PolarCoordinate_tp {
    T r;
    degree theta;

    PolarCoordinate_tp() = default;
    PolarCoordinate_tp(T _r, types::degree _theta) : r(_r), theta(_theta) {}
};
typedef PolarCoordinate_tp<types::astronomical_unit> PolarCoordinate;

struct MoveTarget {
    entt::entity target;
    MoveTarget(entt::entity _targetent) : target(_targetent) {}
};

inline Orbit& updateOrbit(Orbit& orb) {
    orb.theta += orb.angularvelocity;
    return orb;
}

inline void findPeriod(Orbit& orb) { 
    orb.period = TWOPI * std::sqrt(std::pow(orb.semiMajorAxis, 3) / orb.gravitationalparameter);
    orb.angularvelocity = 360.0 * (1.0 / (orb.period * 365));
}

inline types::radian toRadian(types::degree theta) {
    return theta * (cqsp::common::components::types::PI / 180);
}

struct SurfaceCoordinate {
    degree latitude;
    degree longitude;
    float radius;
    entt::entity planet;

    SurfaceCoordinate() = default;
    SurfaceCoordinate(double _lat, double _long, entt::entity _planet)
        : latitude(_lat), longitude(_long), planet(_planet) {
        latitude = toRadian(latitude);
        longitude = toRadian(longitude);
        radius = 0.010;
    }
};

inline types::degree toDegree(types::radian theta) {
    return theta * (180 / cqsp::common::components::types::PI);
}

inline glm::vec3 tovec3(const PolarCoordinate& coordinate) {
    return glm::vec3(coordinate.r * cos(toRadian(coordinate.theta)),
                     0,
                     coordinate.r * sin(toRadian(coordinate.theta)));
}

inline PolarCoordinate toPolarCoordinate(const Orbit& orb) {
    double r = orb.semiMajorAxis * (1 - orb.eccentricity * orb.eccentricity) /
               (1 - orb.eccentricity *
                        cos(toRadian(fmod(orb.theta, 360) + orb.argument)));
    return PolarCoordinate{(types::astronomical_unit)r, fmod(orb.theta, 360)};
}

inline glm::vec3 tovec3(const Orbit& orb) {
    return tovec3(toPolarCoordinate(orb));
}

inline void updatePos(Kinematics& kin, const Orbit& orb) 
{ 
    kin.postion = tovec3(orb); 
}
}  // namespace bodies
}  // namespace components
}  // namespace common
}  // namespace cqsp
