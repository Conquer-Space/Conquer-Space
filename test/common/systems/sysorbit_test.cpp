/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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
#include "common/systems/movement/sysorbit.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <numbers>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "common/actions/maneuver/basicmaneuver.h"
#include "common/actions/maneuver/commands.h"
#include "common/actions/shiplaunchaction.h"
#include "common/components/ships.h"
#include "common/game.h"
#include "common/loading/hjsonloader.h"
#include "common/loading/planetloader.h"
#include "common/simulation.h"
#include "common/util/paths.h"
#include "engine/asset/assetloader.h"
#include "engine/asset/packageindex.h"
#include "engine/asset/vfs/nativevfs.h"

class ManeuverTestSimulation : public cqsp::common::systems::simulation::Simulation {
 public:
    explicit ManeuverTestSimulation(cqsp::common::Game& game) : cqsp::common::systems::simulation::Simulation(game) {}

    void CreateSystems() override { AddSystem<cqsp::common::systems::SysOrbit>(); }
};

struct SysOrbitTest : public ::testing::Test {
 protected:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite() {
        std::filesystem::path data_path(cqsp::common::util::GetCqspTestDataPath());
        std::shared_ptr<cqsp::asset::NativeFileSystem> vfs_shared_ptr = std::make_shared<cqsp::asset::NativeFileSystem>(
            cqsp::asset::NativeFileSystem((data_path / "core").string()));
        // Initialize a few planets
        // Load the core package
        cqsp::asset::PackageIndex index(vfs_shared_ptr->OpenDirectory(""));
        std::string path = index["planets"].path;
        ASSERT_EQ(index["planets"].type, cqsp::asset::AssetType::HJSON);

        planets_hjson = cqsp::asset::LoadHjsonAsset(vfs_shared_ptr, path);
    }

    static void TearDownTestSuite() {}

    void SetUp() override {
        // TODO(EhWhoAmI): If we want further speedup we might be able to move this
        // into the
        cqsp::common::loading::PlanetLoader loader(game.GetUniverse());
        loader.LoadHjson(planets_hjson);

        earth = universe.planets["earth"];
        moon = universe.planets["moon"];

        // 1 tick to initialize the universe
        Tick(1);
    }

    void TearDown() override {}

    static Hjson::Value planets_hjson;

    cqsp::common::Game game;
    cqsp::common::Universe& universe;
    entt::entity earth;
    entt::entity moon;
    ManeuverTestSimulation simulation;

    void Tick(int count = 1) {
        for (int i = 0; i < count; i++) {
            simulation.tick();
        }
    }

    void TickSeconds(double seconds) {
        for (int i = 0; i < std::ceil(seconds / 60.); i++) {
            simulation.tick();
        }
    }

    testing::AssertionResult IsSamePlane(entt::entity ship1, entt::entity ship2, double tolerance = 0.00001) {
        auto& kinematics1 = universe.get<cqsp::common::components::types::Kinematics>(ship1);
        auto& kinematics2 = universe.get<cqsp::common::components::types::Kinematics>(ship2);
        glm::dvec3 angular_momentum1 = glm::cross(kinematics1.position, kinematics1.velocity);
        glm::dvec3 angular_momentum2 = glm::cross(kinematics2.position, kinematics2.velocity);
        double angle = glm::angle(glm::normalize(angular_momentum1), glm::normalize(angular_momentum2));

        if (std::fabs(angle) < tolerance) {
            return testing::AssertionSuccess();
        } else {
            return testing::AssertionFailure()
                   << "The two orbits have a " << angle / std::numbers::pi * 180 << " degree difference";
        }
    }

    SysOrbitTest() : universe(game.GetUniverse()), simulation(game) { simulation.Init(); }
};

Hjson::Value SysOrbitTest::planets_hjson;

TEST_F(SysOrbitTest, BasicOrbitTest) {
    // Let's add something into orbit
    // Let's set this to LEO, at 500 km
    auto& body_component = universe.get<cqsp::common::components::bodies::Body>(earth);

    entt::entity ship1 = cqsp::common::actions::LaunchShip(
        game.GetUniverse(),
        cqsp::common::components::types::Orbit(body_component.radius + 500., 0.00001, 0, 0.01, 0.1, 0, earth));

    Tick(10000);
}

class PlaneMatchTests
    : public SysOrbitTest,
      public testing::WithParamInterface<
          std::tuple<cqsp::common::components::types::Orbit, cqsp::common::components::types::Orbit, double>> {
};  // Chaser orbit, target orbit, and precision

INSTANTIATE_TEST_SUITE_P(
    PlaneMatchingTests, PlaneMatchTests,
    testing::Values(std::tuple(cqsp::common::components::types::Orbit(6371 + 500., 0.00001, 0, 0, 0.1, 0),
                               cqsp::common::components::types::Orbit(6371 + 500., 0.00001, 0.2, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(10000, 0.00001, 0, 0, 0.1, 0),
                               cqsp::common::components::types::Orbit(10000, 0.00001, 0.2, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0.5, 1.5, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 4.5, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 5.4, 5.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(50000, 0.00001, 5.4, 5.8, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(25000, 0.00001, 5.4, 5.8, 0.1, 0),
                               cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0), 0.0001),
                    std::tuple(cqsp::common::components::types::Orbit(7000, 0.00001, 5.4, 5.8, 0.1, 1.3),
                               cqsp::common::components::types::Orbit(380000, 0.4, 0.2, 0.8, 0.1, 0),
                               0.001)));  // This one is not as precise as before, but still is reasonably precise

TEST_P(PlaneMatchTests, MatchPlaneTest) {
    auto& earth_body_component = universe.get<cqsp::common::components::bodies::Body>(earth);
    // Chaser ship
    cqsp::common::components::types::Orbit source_orbit = std::get<0>(GetParam());
    source_orbit.GM = earth_body_component.GM;
    source_orbit.reference_body = earth;
    entt::entity ship1 = cqsp::common::actions::LaunchShip(game.GetUniverse(), source_orbit);

    cqsp::common::components::types::Orbit target_orbit = std::get<1>(GetParam());
    target_orbit.GM = earth_body_component.GM;
    target_orbit.reference_body = earth;
    // Target ship
    entt::entity ship2 = cqsp::common::actions::LaunchShip(game.GetUniverse(), target_orbit);

    cqsp::common::components::Maneuver_t maneuver = cqsp::common::systems::MatchPlanes(source_orbit, target_orbit);
    cqsp::common::systems::commands::PushManeuver(universe, ship1, maneuver);
    ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship1));

    auto& ship1_orbit = universe.get<cqsp::common::components::types::Orbit>(ship1);
    auto& ship2_orbit = universe.get<cqsp::common::components::types::Orbit>(ship2);
    SPDLOG_INFO("Maneuver: {} {}", glm::to_string(maneuver.first), maneuver.second);
    Tick(1);
    SPDLOG_INFO("Chaser orbit: {}", ship1_orbit.ToHumanString());
    SPDLOG_INFO("Target orbit: {}", ship2_orbit.ToHumanString());
    SPDLOG_INFO("Chaser orbit position: {}", glm::to_string(cqsp::common::components::types::toVec3(ship1_orbit)));
    SPDLOG_INFO("Target orbit position: {}", glm::to_string(cqsp::common::components::types::toVec3(ship2_orbit)));

    SPDLOG_INFO("Chaser orbit velocity: {}",
                glm::to_string(cqsp::common::components::types::OrbitVelocityToVec3(ship1_orbit)));
    SPDLOG_INFO("Target orbit velocity: {}",
                glm::to_string(cqsp::common::components::types::OrbitVelocityToVec3(ship2_orbit)));
    auto& queue = universe.get<cqsp::common::components::CommandQueue>(ship1);

    EXPECT_FALSE(queue.maneuvers.empty());
    // Then tick forward, maneuver is in seconds
    Tick((int)(maneuver.second / 60) - 1);
    SPDLOG_INFO("Chaser orbit position: {}", glm::to_string(cqsp::common::components::types::toVec3(ship1_orbit)));
    SPDLOG_INFO("Target orbit position: {}", glm::to_string(cqsp::common::components::types::toVec3(ship2_orbit)));

    SPDLOG_INFO("Chaser orbit velocity: {}",
                glm::to_string(cqsp::common::components::types::OrbitVelocityToVec3(ship1_orbit)));
    SPDLOG_INFO("Target orbit velocity: {}",
                glm::to_string(cqsp::common::components::types::OrbitVelocityToVec3(ship2_orbit)));
    Tick(1);
    // Now compare the planes of the angular momentum

    // Now expect it to still have the similar SMA to the meter
    EXPECT_NEAR(ship1_orbit.semi_major_axis, source_orbit.semi_major_axis, 0.001);
    // Also expect the maneuver queue to be empty
    // TODO(EhWhoAmI): If we have fuel make sure we reduce fuel as well
    ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship1));

    EXPECT_TRUE(queue.maneuvers.empty());

    EXPECT_TRUE(IsSamePlane(ship1, ship2, std::get<2>(GetParam())));

    // Print out the orbits
    SPDLOG_INFO("Chaser orbit: {}", ship1_orbit.ToHumanString());
    SPDLOG_INFO("Target orbit: {}", ship2_orbit.ToHumanString());
    SPDLOG_INFO("Chaser orbit position: {}", glm::to_string(cqsp::common::components::types::toVec3(ship1_orbit)));
    SPDLOG_INFO("Target orbit position: {}", glm::to_string(cqsp::common::components::types::toVec3(ship2_orbit)));

    SPDLOG_INFO("Chaser orbit velocity: {}",
                glm::to_string(cqsp::common::components::types::OrbitVelocityToVec3(ship1_orbit)));
    SPDLOG_INFO("Target orbit velocity: {}",
                glm::to_string(cqsp::common::components::types::OrbitVelocityToVec3(ship2_orbit)));
}

class CircularizeTests : public SysOrbitTest,
                         public testing::WithParamInterface<cqsp::common::components::types::Orbit> {
 protected:
    void ExpectNoPlaneChange(entt::entity ship) {
        cqsp::common::components::types::Orbit source_orbit = GetParam();
        auto& ship_orbit = universe.get<cqsp::common::components::types::Orbit>(ship);
        ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship));

        ASSERT_FALSE(universe.any_of<cqsp::common::components::ships::Crash>(ship));
        // ensure that most of the orbital elements are the same
        EXPECT_NEAR(ship_orbit.inclination, source_orbit.inclination, 1e-6);
        EXPECT_NEAR(ship_orbit.eccentricity, 0, 1e-7);
        EXPECT_NEAR(ship_orbit.LAN, source_orbit.LAN, 1e-6);
    }

    void AddManeuver(cqsp::common::components::Maneuver_t circularize, entt::entity ship) {
        cqsp::common::systems::commands::PushManeuver(universe, ship, circularize);
        ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship));
        TickSeconds(circularize.second + 1.);

        auto& queue = universe.get<cqsp::common::components::CommandQueue>(ship);
        EXPECT_TRUE(queue.maneuvers.empty());
    }

    cqsp::common::components::types::Orbit GetSourceOrbit() {
        auto& earth_body_component = universe.get<cqsp::common::components::bodies::Body>(earth);
        // Chaser ship
        cqsp::common::components::types::Orbit source_orbit = GetParam();
        source_orbit.GM = earth_body_component.GM;
        source_orbit.reference_body = earth;
        return source_orbit;
    }
};

TEST_P(CircularizeTests, ChangeApoapsis) {
    cqsp::common::components::types::Orbit source_orbit = GetSourceOrbit();
    entt::entity ship = cqsp::common::actions::LaunchShip(universe, source_orbit);

    auto& ship_orbit = universe.get<cqsp::common::components::types::Orbit>(ship);
    Tick(1);
    cqsp::common::components::Maneuver_t circularize = cqsp::common::systems::CircularizeAtApoapsis(ship_orbit);
    cqsp::common::systems::commands::PushManeuver(universe, ship, circularize);
    ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship));

    double initial_apoapsis = source_orbit.GetApoapsis();
    TickSeconds(circularize.second + 1.);

    // Now check the new periapsis is the same as the new apoapsis
    EXPECT_NEAR(initial_apoapsis, ship_orbit.GetPeriapsis(), 1.);

    EXPECT_NO_FATAL_FAILURE({ ExpectNoPlaneChange(ship); });
    // Now verify that the amount of delta v we put into it is also the amount we would expect
    // This can be more inaccurate because we are sourcing these values from many different calculations, so we
    // must allow for some numerical inaccuricies
    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(0),
                glm::length(circularize.first) +
                    source_orbit.OrbitalVelocityAtTrueAnomaly(cqsp::common::components::types::apoapsis),
                2);

    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(0),
                cqsp::common::components::types::GetCircularOrbitingVelocity(ship_orbit.GM, ship_orbit.semi_major_axis),
                1);
    EXPECT_LT(ship_orbit.eccentricity, 0.01);
}

TEST_P(CircularizeTests, ChangePeriapsis) {
    cqsp::common::components::types::Orbit source_orbit = GetSourceOrbit();
    entt::entity ship = cqsp::common::actions::LaunchShip(universe, source_orbit);
    // Circularize
    auto& ship_orbit = universe.get<cqsp::common::components::types::Orbit>(ship);
    Tick(1);

    cqsp::common::components::Maneuver_t circularize = cqsp::common::systems::CircularizeAtPeriapsis(ship_orbit);
    cqsp::common::systems::commands::PushManeuver(universe, ship, circularize);
    ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship));

    double initial_periapsis = source_orbit.GetPeriapsis();
    TickSeconds(circularize.second + 1.);

    // Now check the new periapsis is the same as the new apoapsis
    EXPECT_NEAR(initial_periapsis, ship_orbit.GetPeriapsis(), 1.);

    EXPECT_NO_FATAL_FAILURE({ ExpectNoPlaneChange(ship); });

    // Now verify that the amount of delta v we put into it is also the amount we would expect
    // This can be more inaccurate because we are sourcing these values from many different calculations, so we
    // must allow for some numerical inaccuricies
    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(cqsp::common::components::types::apoapsis),
                // Subtract as periapsis operation is reducing orbital energy
                source_orbit.OrbitalVelocityAtTrueAnomaly(0) - glm::length(circularize.first), 2);

    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(cqsp::common::components::types::apoapsis),
                cqsp::common::components::types::GetCircularOrbitingVelocity(ship_orbit.GM, ship_orbit.semi_major_axis),
                1);
    EXPECT_LT(ship_orbit.eccentricity, 0.01);
}

// We shouldn't do a lot of orbits that have no inclination because if we have no inclination we don't really have a difference with
// our LAN.
INSTANTIATE_TEST_SUITE_P(BasicCircularizeTest, CircularizeTests,
                         testing::Values(cqsp::common::components::types::Orbit(10000., 0.2, 0, 0, 0.1, 0),
                                         cqsp::common::components::types::Orbit(10000., 0.3, 0.3, 0, 0.1, 0),
                                         cqsp::common::components::types::Orbit(10000., 0.3, 0.1, 0.3, 0.1, 0),
                                         cqsp::common::components::types::Orbit(10000., 0.3, 0.1, 0.3, 0.1, 1.2),
                                         cqsp::common::components::types::Orbit(20000., 0.6, 0.1, 0.3, 0.1, 1.2),
                                         cqsp::common::components::types::Orbit(50000, 0.2, 2.4, 5.8, 0.1, 1)));

TEST_F(SysOrbitTest, BasicTransferToMoonTest) {
    auto& earth_body_component = universe.get<cqsp::common::components::bodies::Body>(earth);
    // Chaser ship
    cqsp::common::components::types::Orbit source_orbit =
        cqsp::common::components::types::Orbit(50000, 0.00001, 5.4, 5.8, 0.1, 0);
    source_orbit.GM = earth_body_component.GM;
    source_orbit.reference_body = earth;
    entt::entity ship = cqsp::common::actions::LaunchShip(universe, source_orbit);
    Tick(1);
    cqsp::common::systems::commands::TransferToMoon(universe, ship, moon);

    // Check for the maneuvers
    ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship));
    auto& command_queue = universe.get<cqsp::common::components::CommandQueue>(ship);
    // Now check for the circularization
    ASSERT_EQ(command_queue.maneuvers.size(), 1);

    TickSeconds(command_queue.maneuvers.front().time);

    // Then check if our planes are equivalent
    // Check that our plane is the same as the moon...
    // TODO(EhWhoAmI): Fix plane matching so that it's more accurate
    EXPECT_TRUE(IsSamePlane(ship, moon, 0.0174533));  // 1 degree

    // Now compute the maneuver
    ASSERT_EQ(command_queue.maneuvers.size(), 1);
    // The previous maneuver will actually force match planes
    // And add a zero delta v maneuver to actually go to the moon
    TickSeconds(command_queue.maneuvers.front().time);
    // Now we should be going to the moon on the next tick
    if (!command_queue.maneuvers.empty()) {
        TickSeconds(command_queue.maneuvers.front().time);
    }

    // Now we should be on route to the moon
    // Let's check if our apoapsis is intersecting with the moon's orbit
    auto& ship_orbit = universe.get<cqsp::common::components::types::Orbit>(ship);
    auto& moon_orbit = universe.get<cqsp::common::components::types::Orbit>(moon);
    EXPECT_NEAR(ship_orbit.GetApoapsis(), moon_orbit.semi_major_axis, moon_orbit.semi_major_axis * 0.1);
    EXPECT_TRUE(ship_orbit.reference_body == earth);
    // Make sure we hit the moon while we're less than the SOI
    int time = 0;
    double time_to_periapsis = ship_orbit.TimeToTrueAnomaly(0);
    // Check if the time
    while (ship_orbit.reference_body != moon) {
        Tick(1);
        time++;
        // We should be around the moon right now
        ASSERT_LT(time, time_to_periapsis);
    }
    // We should be around the moon
    EXPECT_GT(ship_orbit.eccentricity, 1);

    // Check if we circularize
    ASSERT_EQ(command_queue.maneuvers.size(), 1);
    TickSeconds(command_queue.maneuvers.front().time);

    // Then we should figure out that we are circularized
    // now expect that we have a roughly circular orbit
    EXPECT_LT(ship_orbit.eccentricity, 0.1);
}

TEST_F(SysOrbitTest, DISABLED_EccentricTransferToMoonTest) {
    // Let's add something into orbit
    // Let's set this to LEO, at 500 km
    auto& earth_body_component = universe.get<cqsp::common::components::bodies::Body>(earth);
    // Chaser ship
    cqsp::common::components::types::Orbit source_orbit =
        cqsp::common::components::types::Orbit(50000, 0.2, 2.4, 5.8, 0.1, 1.2);
    source_orbit.GM = earth_body_component.GM;
    source_orbit.reference_body = earth;
    entt::entity ship = cqsp::common::actions::LaunchShip(universe, source_orbit);
    Tick(1);
    cqsp::common::systems::commands::TransferToMoon(universe, ship, moon);

    auto& ship_orbit = universe.get<cqsp::common::components::types::Orbit>(ship);
    auto& moon_orbit = universe.get<cqsp::common::components::types::Orbit>(moon);

    // Check for the maneuvers
    ASSERT_TRUE(universe.all_of<cqsp::common::components::CommandQueue>(ship));
    auto& command_queue = universe.get<cqsp::common::components::CommandQueue>(ship);
    // Now check for circularization
    ASSERT_EQ(command_queue.maneuvers.size(), 1);
    SPDLOG_INFO("Chaser orbit: {}", ship_orbit.ToHumanString());
    TickSeconds(command_queue.maneuvers.front().time);
    SPDLOG_INFO("Chaser orbit: {}", ship_orbit.ToHumanString());

    // Check that our orbit is roughly circular
    EXPECT_LT(ship_orbit.eccentricity, 0.05);

    // Then we wait till the next maneuver
    ASSERT_EQ(command_queue.maneuvers.size(), 1);
    TickSeconds(command_queue.maneuvers.front().time);
    SPDLOG_INFO("Chaser orbit: {}", ship_orbit.ToHumanString());
    ASSERT_EQ(command_queue.maneuvers.size(), 1);
    // Then check if our planes are equivalent
    // Check that our plane is the same as the moon
    EXPECT_TRUE(IsSamePlane(ship, moon, 0.0174533));  // 1 degree

    // The previous maneuver will actually force match planes
    // And add a zero delta v maneuver to actually go to the moon
    TickSeconds(command_queue.maneuvers.front().time);
    // Now we should be going to the moon on the next tick
    TickSeconds(command_queue.maneuvers.front().time);

    // Now we should be on route to the moon
    // Let's check if our apoapsis is intersecting with the moon's orbit
    EXPECT_NEAR(ship_orbit.GetApoapsis(), moon_orbit.semi_major_axis, moon_orbit.semi_major_axis * 0.1);

    // Make sure we hit the moon while we're less than the SOI
    int time = 0;
    double time_to_periapsis = ship_orbit.TimeToTrueAnomaly(0);
    // Check if the time
    while (ship_orbit.reference_body != moon) {
        Tick(1);
        time++;
        // We should be around the moon right now
        ASSERT_LT(time, time_to_periapsis);
    }
    // We should be around the moon
    EXPECT_GT(ship_orbit.eccentricity, 1);

    // Check if we circularize
    ASSERT_EQ(command_queue.maneuvers.size(), 1);
    TickSeconds(command_queue.maneuvers.front().time);

    // Then we should figure out that we are circularized
    // now expect that we have a roughly circular orbit
    EXPECT_LT(ship_orbit.eccentricity, 0.1);
}

TEST_F(SysOrbitTest, ManeuverTest) {
    auto& earth_body_component = universe.get<cqsp::common::components::bodies::Body>(earth);
    // Chaser ship
    cqsp::common::components::types::Orbit source_orbit =
        cqsp::common::components::types::Orbit(50000, 0.2, 2.4, 5.8, 0.1, 1);
    source_orbit.GM = earth_body_component.GM;
    source_orbit.reference_body = earth;
    const entt::entity ship = cqsp::common::actions::LaunchShip(universe, source_orbit);

    // Let's just add prograde maneuvers everywhere and see if we see the intended velocity changes
    const auto maneuver = cqsp::common::systems::commands::MakeManeuver(glm::dvec3(0, 2, 0), 1000);
    cqsp::common::systems::commands::PushManeuver(universe, ship, maneuver);

    const auto& ship_orbit = universe.get<cqsp::common::components::types::Orbit>(ship);

    // Let's get the expected speed at the true anomaly
    const double true_anomaly = cqsp::common::components::types::GetTrueAnomaly(ship_orbit, maneuver.second);
    // Now get our value
    glm::dvec3 orbital_vector = OrbitVelocityToVec3(ship_orbit, true_anomaly);

    // Now go ahead in time
    TickSeconds(maneuver.second);
    // Now check our orbit
    // Now get our true anomaly at time
    const double new_anomaly = cqsp::common::components::types::GetTrueAnomaly(ship_orbit, maneuver.second);
    // Now check our velocity
    glm::dvec3 new_orbital_vector = OrbitVelocityToVec3(ship_orbit, new_anomaly);
    EXPECT_NEAR(glm::length(orbital_vector - new_orbital_vector), 2, 0.001);

    SPDLOG_INFO("Final vector difference: {}", glm::to_string(orbital_vector - new_orbital_vector));
}
