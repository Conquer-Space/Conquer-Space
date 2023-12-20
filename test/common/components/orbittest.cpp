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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <hjson.h>

#include <fstream>

#include "common/components/coordinates.h"
#include "common/components/orbit.h"
#include "common/components/units.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/movement/sysmovement.h"
#include "common/universe.h"

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;

// Tests for input from client options
TEST(OrbitTest, DISABLED_toVec3Test) {
    // Read hjson file and set values
    Hjson::Value data = Hjson::UnmarshalFromFile("data_file.hjson");
    // Do the test
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    double a = orb.semi_major_axis = data["semi_major_axis"];
    double e = orb.eccentricity = data["eccentricity"];
    double i = orb.inclination = data["inclination"];
    double LAN = orb.LAN = data["ascending_node"];
    double w = orb.w = data["argument"];
    //std::cout << orb.T() << std::endl;
    double T = 3.1415926535 * 2;
    int resolution = 5000;
    std::ofstream file("data.txt");
    for (int i = 0; i < resolution + 1; i++) {
        glm::vec3 vec = cqspt::OrbitToVec3(a, e, i, LAN, w, T / resolution * i);
        std::cout.precision(17);
        file << vec.x << " " << vec.y << " " << vec.z << std::endl;
        //EXPECT_THAT(glm::length(vec), AllOf(Ge(0.98326934275),Le(1.0167257013)));
    }
    file.close();
    EXPECT_NEAR(orb.T() / 86400, 365.256363004, 0.01);
}

TEST(OrbitTest, OrbitConversionTest) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0;
    orb.inclination = 0;
    orb.LAN = 0;
    orb.w = 0;

    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be 0
    EXPECT_EQ(orb.GetMtElliptic(0), 0);
    EXPECT_EQ(orb.v, 0);
    EXPECT_EQ(orb.E, 0);
    EXPECT_EQ(orb.M0, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_NEAR(glm::length(velocity), cqspt::AvgOrbitalVelocity(orb), cqspt::AvgOrbitalVelocity(orb) * 0.0001);
    EXPECT_NEAR(glm::length(position), orb.semi_major_axis, orb.semi_major_axis * 0.001);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.GM, 0);
    // We need the fmod things because the orbital elements are a bit wacky with perfectly round and non-inclined orbits
    EXPECT_DOUBLE_EQ(fmod(new_orbit.v, cqspt::PI), std::fmod(orb.v, cqspt::PI));
    EXPECT_DOUBLE_EQ(fmod(new_orbit.E, cqspt::PI), std::fmod(orb.E, cqspt::PI));
    EXPECT_DOUBLE_EQ(fmod(new_orbit.M0, cqspt::PI), std::fmod(orb.M0, cqspt::PI));
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis, orb.semi_major_axis * 0.01);
    EXPECT_DOUBLE_EQ(new_orbit.LAN, orb.LAN);
    EXPECT_DOUBLE_EQ(new_orbit.inclination, orb.inclination);
    EXPECT_DOUBLE_EQ(new_orbit.w, orb.w);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
}

TEST(OrbitTest, NewOrbitConversionTest) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.6;
    orb.inclination = 0;
    orb.LAN = 0;
    orb.w = 0;

    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be 0
    EXPECT_EQ(orb.GetMtElliptic(0), 0);
    EXPECT_EQ(orb.v, 0);
    EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_DOUBLE_EQ(glm::length(position), cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v));
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.GM, 0);
    EXPECT_DOUBLE_EQ(new_orbit.v, orb.v);
    EXPECT_DOUBLE_EQ(new_orbit.E, orb.E);
    EXPECT_DOUBLE_EQ(new_orbit.M0, orb.M0);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis, orb.semi_major_axis * 0.01);
    EXPECT_DOUBLE_EQ(new_orbit.LAN, orb.LAN);
    EXPECT_DOUBLE_EQ(new_orbit.inclination, orb.inclination);
    EXPECT_DOUBLE_EQ(new_orbit.w, orb.w);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
}

TEST(OrbitTest, NewOrbitConversionTest2) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.6;
    orb.inclination = 0;
    orb.LAN = 0;
    orb.w = 0;
    orb.M0 = 0.8;

    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be 0.8
    EXPECT_EQ(orb.GetMtElliptic(0), 0.8);
    //EXPECT_EQ(orb.v, 0);
    //EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_NEAR(glm::length(position), cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
                orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.GM, 0);
    EXPECT_NEAR(new_orbit.v, orb.v, 0.001);
    EXPECT_NEAR(new_orbit.E, orb.E, 0.001);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 0.001);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.01);  // 1 % cause doubles are bad
    EXPECT_NEAR(new_orbit.LAN, orb.LAN, 0.001);
    EXPECT_NEAR(new_orbit.inclination, orb.inclination, 0.001);
    EXPECT_NEAR(new_orbit.w, orb.w, 0.001);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 500);
        EXPECT_NEAR(new_pos.y, position.y, 500);
        EXPECT_NEAR(new_pos.z, position.z, 500);
    }
}

// Disabled for now because it doesn't work on gcc for some godforsaken reason
TEST(OrbitTest, DISABLED_NewOrbitConversionTest3) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0;
    orb.inclination = 1;
    orb.LAN = 0;
    orb.w = 0;
    double M0 = 0;
    orb.M0 = M0;

    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be M0
    EXPECT_EQ(orb.GetMtElliptic(0), M0);
    EXPECT_EQ(orb.v, 0);
    EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    EXPECT_NEAR(position.z, 0, 0.001);
    EXPECT_NEAR(position.y, 0, 0.001);
    EXPECT_NEAR(position.x, orb.semi_major_axis, 200);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_EQ(acos(1), 0);
    EXPECT_NEAR(glm::length(position), cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
                orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.GM, 0);
    EXPECT_NEAR(new_orbit.v, orb.v, 0.001);
    EXPECT_NEAR(new_orbit.E, orb.E, 0.001);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 0.001);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.0001);  // 0.01% error cause doubles are bad

    // It's fine if it's 2 pi for this test, because it's a full circle
    EXPECT_NEAR(std::fmod(new_orbit.LAN + new_orbit.w, 2 * cqspt::PI), 0, 0.001);
    EXPECT_NEAR(new_orbit.inclination, orb.inclination, 0.001);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 0.001);
        EXPECT_NEAR(new_pos.y, position.y, 0.001);
        EXPECT_NEAR(new_pos.z, position.z, 0.001);
    }
}

TEST(OrbitTest, NewOrbitConversionTest4) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.1;
    orb.inclination = 0.1;
    orb.LAN = 0.2;
    orb.w = 0.7;
    double M0 = cqspt::PI / 4;
    orb.M0 = M0;

    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be M0
    EXPECT_EQ(orb.GetMtElliptic(0), M0);
    // EXPECT_EQ(orb.v, 0);
    // EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);

    EXPECT_EQ(acos(1), 0);
    EXPECT_NEAR(glm::length(position), cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
                orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.GM, 0);
    EXPECT_NEAR(new_orbit.v, orb.v, 0.001);
    EXPECT_NEAR(new_orbit.E, orb.E, 0.001);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 0.001);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.01);  // 1% error cause doubles are bad
    EXPECT_NEAR(new_orbit.LAN, orb.LAN, 0.001);
    EXPECT_NEAR(new_orbit.w, orb.w, 0.001);
    EXPECT_NEAR(new_orbit.inclination, orb.inclination, 0.001);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
    EXPECT_NEAR(new_orbit.v, orb.v, 0.001);
    EXPECT_NEAR(new_orbit.E, orb.E, 0.001);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 0.001);
    auto new_pos = cqspt::toVec3(new_orbit);
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 0.001);
        EXPECT_NEAR(new_pos.y, position.y, 0.001);
        EXPECT_NEAR(new_pos.z, position.z, 0.001);
    }
}

TEST(OrbitTest, NewOrbitConversionTest5) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.9;
    orb.inclination = 3.14;
    orb.LAN = 0.29;
    orb.w = 0.68;
    cqspt::radian M0 = 2.8;
    orb.M0 = M0;

    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be M0
    EXPECT_EQ(orb.GetMtElliptic(0), M0);
    // EXPECT_EQ(orb.v, 0);
    // EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);

    EXPECT_EQ(acos(1), 0);
    EXPECT_NEAR(glm::length(position), cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
                orb.semi_major_axis * 1e-5);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.GM, 0);
    EXPECT_NEAR(new_orbit.v, orb.v, 1e-5);
    EXPECT_NEAR(new_orbit.E, orb.E, 1e-5);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 1e-5);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                1e-5);  // 1m difference
    EXPECT_NEAR(new_orbit.LAN, orb.LAN, 1e-5);
    EXPECT_NEAR(new_orbit.w, orb.w, 1e-5);
    EXPECT_NEAR(new_orbit.inclination, orb.inclination, 1e-5);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 1e-5);
    EXPECT_NEAR(new_orbit.v, orb.v, 1e-5);
    EXPECT_NEAR(new_orbit.E, orb.E, 1e-5);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 1e-5);

    auto new_pos = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(new_pos.x, position.x, 1e-4);
    EXPECT_NEAR(new_pos.y, position.y, 1e-4);
    EXPECT_NEAR(new_pos.z, position.z, 1e-4);
    // Check all the points of the orbit
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        auto new_velocity = cqspt::OrbitVelocityToVec3(new_orbit, cqspt::toRadian(i));
        auto velocity = cqspt::OrbitVelocityToVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 1e-4);
        EXPECT_NEAR(new_pos.y, position.y, 1e-4);
        EXPECT_NEAR(new_pos.z, position.z, 1e-4);
        EXPECT_NEAR(new_velocity.x, velocity.x, 1e-4);
        EXPECT_NEAR(new_velocity.y, velocity.y, 1e-4);
        EXPECT_NEAR(new_velocity.z, velocity.z, 1e-4);
        // Check for the tangental orbital velocity
        double t_velocity = cqspt::OrbitVelocity(cqspt::toRadian(i), orb.eccentricity, orb.semi_major_axis, orb.GM);
        EXPECT_NEAR(glm::length(velocity), t_velocity, 1e-4);
    }
}

TEST(OrbitTest, OrbitImpulseTest) {
    namespace cqspt = cqsp::common::components::types;
    // Make a random orbit, apply an impulse, and ensure the position is te same
    cqspt::Orbit orbit(57.91e7, 0.1, 1.45, 0.29, 0.68, 0);
    double velocity = cqspt::OrbitVelocity(orbit.v, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
    glm::dvec3 impulse(0, 1, 0);
    cqspt::Orbit new_orbit = cqspt::ApplyImpulse(orbit, impulse, 0);

    // Sanity check on the vector addition
    EXPECT_NEAR(glm::length(cqspt::ConvertToOrbitalVector(orbit.LAN, orbit.inclination, orbit.w, orbit.v, impulse)), 1,
                0.00001);
    // Dot product the velociy of the orbit should be 0
    glm::dvec3 transfer_vec = cqspt::ConvertToOrbitalVector(orbit.LAN, orbit.inclination, orbit.w, orbit.v, impulse);
    glm::dvec3 velocity_vec = cqspt::OrbitVelocityToVec3(orbit, 0);
    // The velocity vector and transfer vector should be parallel
    EXPECT_NEAR(glm::dot(glm::normalize(transfer_vec), glm::normalize(velocity_vec)), 1, 1e-4);
    double r = orbit.GetOrbitingRadius();
    double r2 = new_orbit.GetOrbitingRadius();
    EXPECT_NEAR(r, r2, 1e-4);
    glm::dvec3 orbit_vec = cqspt::toVec3(orbit);
    glm::dvec3 new_orbit_vec = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(orbit_vec.x, new_orbit_vec.x, 1e-4);
    EXPECT_NEAR(orbit_vec.y, new_orbit_vec.y, 1e-4);
    EXPECT_NEAR(orbit_vec.z, new_orbit_vec.z, 1e-4);
    // SMA should be higher because we are burning prograde
    EXPECT_GT(new_orbit.semi_major_axis, orbit.semi_major_axis);
    EXPECT_GT(new_orbit.eccentricity, orbit.eccentricity);

    // Checks if their position matches up with what we expect it to be
    glm::dvec3 orbit_v = cqspt::OrbitVelocityToVec3(orbit, 0);
    glm::dvec3 new_orbit_v = cqspt::OrbitVelocityToVec3(new_orbit, 0);

    EXPECT_GT(glm::length(new_orbit_v), glm::length(orbit_v));
    EXPECT_NEAR(glm::length(new_orbit_v) - glm::length(orbit_v), 1, 1e-4);

    // Check out the new semi major axis
    // velocity + glm::length(impulse)
    double orb_velocity = velocity + glm::length(impulse);
    double new_sma = -1 / (orb_velocity * orb_velocity / orbit.GM - 2 / glm::length(orbit_vec));
    EXPECT_NEAR(new_orbit.semi_major_axis, new_sma, 1e-4);
    // Eccentricity should be
    double ap = new_sma * 2 - glm::length(orbit_vec);
    double pe = glm::length(orbit_vec);
    double ecc = (ap - pe) / (ap + pe);
    EXPECT_NEAR(ecc, new_orbit.eccentricity, 1e-10);
    EXPECT_NEAR(orbit.inclination, new_orbit.inclination, 1e-10);
    EXPECT_NEAR(orbit.w, new_orbit.w, 1e-10);
    EXPECT_NEAR(orbit.LAN, new_orbit.LAN, 1e-10);
}

TEST(OrbitTest, ToRadianTest) {
    namespace cqspt = cqsp::common::components::types;
    EXPECT_DOUBLE_EQ(cqspt::PI / 2, cqspt::toRadian(90));
    EXPECT_DOUBLE_EQ(cqspt::PI, cqspt::toRadian(180));
    EXPECT_DOUBLE_EQ(cqspt::PI * 2.f, cqspt::toRadian(360));
    EXPECT_DOUBLE_EQ(cqspt::PI / 6, cqspt::toRadian(30));
    EXPECT_DOUBLE_EQ(cqspt::PI / 3, cqspt::toRadian(60));
    EXPECT_DOUBLE_EQ(cqspt::PI / 4, cqspt::toRadian(45));
}

TEST(OrbitTest, ToDegreeTest) {
    namespace cqspt = cqsp::common::components::types;
    EXPECT_DOUBLE_EQ(30, cqspt::toDegree(cqspt::PI / 6));
    EXPECT_DOUBLE_EQ(45, cqspt::toDegree(cqspt::PI / 4));
    EXPECT_DOUBLE_EQ(60, cqspt::toDegree(cqspt::PI / 3));
    EXPECT_DOUBLE_EQ(90, cqspt::toDegree(cqspt::PI / 2));
    EXPECT_DOUBLE_EQ(180, cqspt::toDegree(cqspt::PI));
    EXPECT_DOUBLE_EQ(360, cqspt::toDegree(cqspt::PI * 2));
}

TEST(OrbitTest, CircularOrbitVelocityTest) {
    // Make circular Orbit
    using namespace cqsp::common::components::types;  // NOLINT
    Orbit orbit(150000000, 0, 0, 0, 0, 0);
    double v = GetCircularOrbitingVelocity(SunMu, 150000000);
    double orb_v = glm::length(OrbitVelocityToVec3(orbit, 0));
    EXPECT_DOUBLE_EQ(v, orb_v);
}

TEST(OrbitTest, EllipticOrbitTest) {
    // Make circular Orbit
    using namespace cqsp::common::components::types;  // NOLINT
    Orbit orbit(150000000, 0.5, 0, 0, 0, 0);
    for (int i = 0; i < 360; i++) {
        double v = (double)(i / (PI * 2));
        double calc_velocity = OrbitVelocity(v, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
        double E = SolveKeplerElliptic(orbit.eccentricity, v);
        //EXPECT_DOUBLE_EQ(v, E - orbit.eccentricity * )
        double orb_velocity = glm::length(OrbitVelocityToVec3(orbit, v));
        EXPECT_DOUBLE_EQ(calc_velocity, orb_velocity);
        // Check orbit altitude?
        double rad = GetOrbitingRadius(orbit.eccentricity, orbit.semi_major_axis, v);
        double len = glm::length(toVec3(orbit, v));
        EXPECT_DOUBLE_EQ(rad, len);
    }
}

TEST(OrbitTest, SolveKeplerHyperbolic) {
    // https://www.fxsolver.com/browse/formulas/Hyperbolic+Kepler+equation
    std::vector<std::tuple<double, double, double>> data = {
        {10, 1.5, 2.84394720242}, {2, 1.5, 1.61268580976}, {6, 2.5, 1.86344302689}};
    for (auto &line : data) {
        EXPECT_NEAR(cqsp::common::components::types::SolveKeplerHyperbolic(std::get<0>(line), std::get<1>(line)),
                    std::get<2>(line), 1e-6);
    }
}

TEST(OrbitTest, SolveKeplerElliptic) {
    // https://www.fxsolver.com/browse/formulas/Kepler%27s+equation
    std::vector<std::tuple<double, double, double>> data = {
        {2, 0.3, 2.23603149517}, {1.5, 0.8, 2.16353230394}, {0.5, 0.01, 0.504836644695}};
    for (auto &line : data) {
        EXPECT_NEAR(cqsp::common::components::types::SolveKeplerElliptic(std::get<0>(line), std::get<1>(line)),
                    std::get<2>(line), 1e-6);
    }
}

TEST(Common_TransferTest, TransferTimeTest_Mars) {
    namespace cqspt = cqsp::common::components::types;
    using namespace cqspt;  // NOLINT
    cqspt::Orbit earth_orbit;
    earth_orbit.semi_major_axis = 149598023;
    earth_orbit.v = 356.9521225619375_deg;
    earth_orbit.GM = cqspt::SunMu;

    cqspt::Orbit mars_orbit;
    mars_orbit.semi_major_axis = 227939366;
    mars_orbit.GM = cqspt::SunMu;
    mars_orbit.v = 0.338803314_deg;

    double b = cqspt::CalculateTransferTime(earth_orbit, mars_orbit);
    double p = cqspt::CalculateTransferAngle(earth_orbit, mars_orbit);
    // The approximate transfer time between the two
    EXPECT_NEAR(b / 86400, 259, 2);
}

TEST(OrbitTest, OrbitNormalTest) {
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit vec = {57.91e9, 0., 0.0, 0., 0., 0};
    glm::dvec3 orb = cqspt::GetOrbitNormal(vec);
    glm::dvec3 vel = glm::cross(cqspt::toVec3(vec), cqspt::OrbitVelocityToVec3(vec, 0));
    EXPECT_EQ(glm::dot(glm::normalize(orb), glm::normalize(vel)), 1.);
}

TEST(OrbitTest, OrbitNormalTest2) {
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit vec = {57.91e9, 0.3, 0.4, 0., 0., 0};
    glm::dvec3 orb = cqspt::GetOrbitNormal(vec);
    glm::dvec3 vel = glm::cross(cqspt::toVec3(vec), cqspt::OrbitVelocityToVec3(vec, 0));
    EXPECT_EQ(glm::dot(glm::normalize(orb), glm::normalize(vel)), 1.);
}

TEST(OrbitTest, AscendingNodeTest) {
    namespace cqspt = cqsp::common::components::types;
    std::vector<std::tuple<cqspt::Orbit, cqspt::Orbit, double>> map = {
        {{57.91e9, 0., 0.0, 0., 0., 0}, {57.91e9, 0., 0.5, 0., 0, 0}, 0},
        {{57.91e9, 0., 0.0, 0., 0., 0}, {57.91e9, 0., 0.5, 0., cqspt::PI, 0}, 0},
        {{57.91e9, 0., 0.5, 0., cqspt::PI / 2, 0}, {57.91e9, 0., 0.5, 0., 0.0, 0}, cqspt::PI / 2},
        //{{57.91e9, 0., 0.4, 0.3, 0., 0}, {57.91e9, 0., 0.5, 0.2, 0, 0}, 5.82736}
    };
    for (auto &element : map) {
        EXPECT_DOUBLE_EQ(cqspt::AscendingTrueAnomaly(std::get<0>(element), std::get<1>(element)), std::get<2>(element));
    }
}

TEST(OrbitTest, PhaseAngleTest) {
    namespace cqspt = cqsp::common::components::types;
    // The angles are taken from https://ksp.olex.biz/, which should be a decent estimation.
    cqspt::Orbit kerbin;
    kerbin.semi_major_axis = 13599840.256;
    kerbin.GM = 1.1723328e9;

    cqspt::Orbit duna;
    duna.GM = 1.1723328e9;
    duna.semi_major_axis = 20726155.264;

    EXPECT_NEAR(cqspt::CalculateTransferAngle(kerbin, duna), cqspt::toRadian(44.6), 0.5);
    // Eve transfer
    cqspt::Orbit eve;
    eve.GM = 1.1723328e9;
    eve.semi_major_axis = 9832684.544;

    EXPECT_NEAR(cqspt::CalculateTransferAngle(kerbin, eve), cqspt::toRadian(-54.13), 0.5);
}
/*
TEST(Common_SOITest, SOIExitTest) {
    namespace cqspc = cqsp::common::components;
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::systems;
    cqsp::common::Universe universe;
    // Make bodies
    entt::entity body_1 = universe.create();
    entt::entity body_2 = universe.create();
    entt::entity satellite = universe.create();
    universe.emplace<cqspt::Orbit>(body_1);
    universe.emplace<cqspt::Kinematics>(body_1);
    universe.emplace<cqspc::bodies::Body>(body_1);
    auto& body1_orb = universe.emplace<cqspc::bodies::OrbitalSystem>(body_1);
    universe.emplace<cqspt::Orbit>(body_2).reference_body = body_1;
    universe.emplace<cqspt::Kinematics>(body_2);
    auto& body2_orb = universe.emplace<cqspc::bodies::OrbitalSystem>(body_2);
    body1_orb.push_back(body_2);
    body2_orb.push_back(satellite);
    universe.emplace<cqspt::Orbit>(satellite).reference_body = body_2;
    universe.emplace<cqspt::Kinematics>(satellite);
    // Add all the necessary thigns
    EXPECT_EQ(body1_orb.children.size(), 1);
    EXPECT_EQ(body2_orb.children.size(), 1);
    cqsps::LeaveSOI(universe, satellite);
    EXPECT_EQ(body1_orb.children.size(), 2);
    EXPECT_EQ(body1_orb.children[0], body_2);
    EXPECT_EQ(body1_orb.children[1], satellite);
    EXPECT_EQ(body2_orb.children.size(), 0);
}
 */
