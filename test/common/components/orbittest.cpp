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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <hjson.h>
#include <fstream>
#include "common/components/coordinates.h"
#include "common/universe.h"
#include "common/systems/movement/sysmovement.h"

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;

// Tests for input from client options
TEST(Common_OrbitTest, DISABLED_toVec3Test) {
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
    orb.CalculateVariables();
    std::cout << orb.T << std::endl;
    double T = 3.1415926535 * 2;
    int resolution = 5000;
    std::ofstream file("data.txt");
    for (int i = 0; i < resolution + 1; i++) {
        glm::vec3 vec = cqspt::OrbitToVec3(a, e, i, LAN, w, T/resolution * i);
        std::cout.precision(17);
        file << vec.x << " " << vec.y << " " << vec.z
             << std::endl;
        //EXPECT_THAT(glm::length(vec), AllOf(Ge(0.98326934275),Le(1.0167257013)));
    }
    file.close();
    EXPECT_NEAR(orb.T/86400, 365.256363004, 0.01);
}

TEST(Common_OrbitTest, OrbitConversionTest) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0;
    orb.inclination = 0;
    orb.LAN = 0;
    orb.w = 0;

    orb.CalculateVariables();
    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be 0
    EXPECT_EQ(orb.GetMt(0), 0);
    EXPECT_EQ(orb.v, 0);
    EXPECT_EQ(orb.E, 0);
    EXPECT_EQ(orb.M0, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_NEAR(glm::length(velocity), cqspt::AvgOrbitalVelocity(orb),
                cqspt::AvgOrbitalVelocity(orb) * 0.0001);
    EXPECT_NEAR(glm::length(position), orb.semi_major_axis, orb.semi_major_axis * 0.001);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.Mu, 0);
    EXPECT_DOUBLE_EQ(new_orbit.v, orb.v);
    EXPECT_DOUBLE_EQ(new_orbit.E, orb.E);
    EXPECT_DOUBLE_EQ(new_orbit.M0, orb.M0);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.01);
    EXPECT_DOUBLE_EQ(new_orbit.LAN, orb.LAN);
    EXPECT_DOUBLE_EQ(new_orbit.inclination, orb.inclination);
    EXPECT_DOUBLE_EQ(new_orbit.w, orb.w);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
}

TEST(Common_OrbitTest, NewOrbitConversionTest) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.6;
    orb.inclination = 0;
    orb.LAN = 0;
    orb.w = 0;

    orb.CalculateVariables();
    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be 0
    EXPECT_EQ(orb.GetMt(0), 0);
    EXPECT_EQ(orb.v, 0);
    EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_DOUBLE_EQ(
        glm::length(position),
        cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v));
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.Mu, 0);
    EXPECT_DOUBLE_EQ(new_orbit.v, orb.v);
    EXPECT_DOUBLE_EQ(new_orbit.E, orb.E);
    EXPECT_DOUBLE_EQ(new_orbit.M0, orb.M0);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.01);
    EXPECT_DOUBLE_EQ(new_orbit.LAN, orb.LAN);
    EXPECT_DOUBLE_EQ(new_orbit.inclination, orb.inclination);
    EXPECT_DOUBLE_EQ(new_orbit.w, orb.w);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
}

TEST(Common_OrbitTest, NewOrbitConversionTest2) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.6;
    orb.inclination = 0;
    orb.LAN = 0;
    orb.w = 0;
    orb.M0 = 0.8;

    orb.CalculateVariables();
    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be 0.8
    EXPECT_EQ(orb.GetMt(0), 0.8);
    //EXPECT_EQ(orb.v, 0);
    //EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_NEAR(
        glm::length(position),
        cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
        orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.Mu, 0);
    EXPECT_NEAR(new_orbit.v, orb.v, 0.001);
    EXPECT_NEAR(new_orbit.E, orb.E, 0.001);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 0.001);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.01); // 1 % cause doubles are bad
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

TEST(Common_OrbitTest, NewOrbitConversionTest3) {
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

    orb.CalculateVariables();
    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be M0
    EXPECT_EQ(orb.GetMt(0), M0);
    // EXPECT_EQ(orb.v, 0);
    // EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);
    EXPECT_EQ(acos(1), 0);
    EXPECT_NEAR(
        glm::length(position),
        cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
        orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.Mu, 0);
    EXPECT_NEAR(new_orbit.v, orb.v, 0.001);
    EXPECT_NEAR(new_orbit.E, orb.E, 0.001);
    EXPECT_NEAR(new_orbit.M0, orb.M0, 0.001);
    EXPECT_NEAR(new_orbit.semi_major_axis, orb.semi_major_axis,
                orb.semi_major_axis * 0.01);  // 1% error cause doubles are bad
    EXPECT_NEAR(std::fmod(new_orbit.LAN + new_orbit.w, 2 * cqspt::PI), 0, 0.001); // It's fine if it's 2 pi for this test, because it's a full circle
    EXPECT_NEAR(new_orbit.inclination, orb.inclination, 0.001);
    EXPECT_NEAR(new_orbit.eccentricity, orb.eccentricity, 0.001);
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 500);
        EXPECT_NEAR(new_pos.y, position.y, 500);
        EXPECT_NEAR(new_pos.z, position.z, 500);
    }
}

TEST(Common_OrbitTest, NewOrbitConversionTest4) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.1;
    orb.inclination = 0.1;
    orb.LAN = 0.2;
    orb.w = 0.7;
    double M0 = cqspt::PI/4;
    orb.M0 = M0;

    orb.CalculateVariables();
    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be M0
    EXPECT_EQ(orb.GetMt(0), M0);
    // EXPECT_EQ(orb.v, 0);
    // EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);

    EXPECT_EQ(acos(1), 0);
    EXPECT_NEAR(
        glm::length(position),
        cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
        orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.Mu, 0);
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
    new_orbit.CalculateVariables();
    auto new_pos = cqspt::toVec3(new_orbit);
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 300);
        EXPECT_NEAR(new_pos.y, position.y, 300);
        EXPECT_NEAR(new_pos.z, position.z, 300);
    }
}

TEST(Common_OrbitTest, NewOrbitConversionTest5) {
    // Expect the orbit is similar
    namespace cqspt = cqsp::common::components::types;
    cqspt::Orbit orb;
    orb.semi_major_axis = 57.91e7;
    orb.eccentricity = 0.9;
    orb.inclination = 3.14;
    orb.LAN = 0.29;
    orb.w = 0.68;
    double M0 = 2.8;
    orb.M0 = M0;

    orb.CalculateVariables();
    cqspt::UpdateOrbit(orb, 0);
    // Expect the true anomaly to be M0
    EXPECT_EQ(orb.GetMt(0), M0);
    // EXPECT_EQ(orb.v, 0);
    // EXPECT_EQ(orb.E, 0);
    auto position = cqspt::toVec3(orb);
    auto velocity = cqspt::OrbitVelocityToVec3(orb, orb.v);

    EXPECT_EQ(acos(1), 0);
    EXPECT_NEAR(
        glm::length(position),
        cqspt::GetOrbitingRadius(orb.eccentricity, orb.semi_major_axis, orb.v),
        orb.semi_major_axis * 0.01);
    auto new_orbit = cqspt::Vec3ToOrbit(position, velocity, orb.Mu, 0);
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
    new_orbit.CalculateVariables();
    auto new_pos = cqspt::toVec3(new_orbit);
    EXPECT_NEAR(new_pos.x, position.x, 300);
    EXPECT_NEAR(new_pos.y, position.y, 300);
    EXPECT_NEAR(new_pos.z, position.z, 300);
    // Check all the points of the orbit
    for (int i = 0; i < 360; i++) {
        auto new_pos = cqspt::toVec3(new_orbit, cqspt::toRadian(i));
        auto position = cqspt::toVec3(orb, cqspt::toRadian(i));
        EXPECT_NEAR(new_pos.x, position.x, 300);
        EXPECT_NEAR(new_pos.y, position.y, 300);
        EXPECT_NEAR(new_pos.z, position.z, 300);
    }
}

TEST(Common_OrbitTest, ToRadianTest) {
    namespace cqspt = cqsp::common::components::types;
    EXPECT_DOUBLE_EQ(cqspt::PI/2, cqspt::toRadian(90));
    EXPECT_DOUBLE_EQ(cqspt::PI, cqspt::toRadian(180));
    EXPECT_DOUBLE_EQ(cqspt::PI * 2.f, cqspt::toRadian(360));
    EXPECT_DOUBLE_EQ(cqspt::PI/6, cqspt::toRadian(30));
    EXPECT_DOUBLE_EQ(cqspt::PI/3, cqspt::toRadian(60));
    EXPECT_DOUBLE_EQ(cqspt::PI/4, cqspt::toRadian(45));
}

TEST(Common_OrbitTest, ToDegreeTest) {
    namespace cqspt = cqsp::common::components::types;
    EXPECT_DOUBLE_EQ(30, cqspt::toDegree(cqspt::PI/6));
    EXPECT_DOUBLE_EQ(45, cqspt::toDegree(cqspt::PI/4));
    EXPECT_DOUBLE_EQ(60, cqspt::toDegree(cqspt::PI/3));
    EXPECT_DOUBLE_EQ(90, cqspt::toDegree(cqspt::PI/2));
    EXPECT_DOUBLE_EQ(180, cqspt::toDegree(cqspt::PI));
    EXPECT_DOUBLE_EQ(360, cqspt::toDegree(cqspt::PI*2));
}

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
