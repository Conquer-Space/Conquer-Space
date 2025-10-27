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

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "common/actions/maneuver/commands.h"
#include "common/actions/maneuver/maneuver.h"
#include "common/actions/shiplaunchaction.h"
#include "common/game.h"
#include "common/loading/hjsonloader.h"
#include "common/loading/planetloader.h"
#include "common/simulation.cpp"
#include "common/util/paths.h"
#include "engine/asset/packageindex.h"
#include "engine/asset/vfs/nativevfs.h"

class ManeuverTestSimulation : public cqsp::common::systems::simulation::Simulation {
 public:
    explicit ManeuverTestSimulation(cqsp::common::Game& game) : cqsp::common::systems::simulation::Simulation(game) {}

    void CreateSystems() override { AddSystem<cqsp::common::systems::SysOrbit>(); }
};

Hjson::Value LoadHjsonAsset(cqsp::asset::IVirtualFileSystemPtr mount, std::string path) {
    Hjson::Value value;
    Hjson::DecoderOptions dec_opt;
    dec_opt.comments = false;

    if (mount->IsDirectory(path)) {
        // Load and append to assets.
        auto dir = mount->OpenDirectory(path);
        for (int i = 0; i < dir->GetSize(); i++) {
            auto file = dir->GetFile(i);
            Hjson::Value result;
            // Since it's a directory, we will assume it's an array, and push back the values.
            try {
                result = Hjson::Unmarshal(ReadAllFromVFileToString(file.get()), dec_opt);
                if (result.type() == Hjson::Type::Vector) {
                    // Append all the values in place
                    for (int k = 0; k < result.size(); k++) {
                        value.push_back(result[k]);
                    }
                } else {
                    // TODO(EhWhoAmI): Raise a non fatal error
                }
            } catch (Hjson::syntax_error& ex) {
                // TODO(EhWhoAmI): Also raise a non fatal error
            }
        }
    } else {
        auto file = mount->Open(path);
        // Read the file
        try {
            value = Hjson::Unmarshal(ReadAllFromVFileToString(file.get()), dec_opt);
        } catch (Hjson::syntax_error& ex) {
            // TODO(EhWhoAmI): Raise a fatal error
        }
    }
    return value;
}

struct SysOrbitTest : public ::testing::Test {
 protected:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite() {
        std::filesystem::path data_path(cqsp::common::util::GetCqspTestDataPath());
        std::shared_ptr<cqsp::asset::NativeFileSystem> vfs_shared_ptr = std::shared_ptr<cqsp::asset::NativeFileSystem>(
            new cqsp::asset::NativeFileSystem((data_path / "core").string()));
        // Initialize a few planets
        // Load the core package
        cqsp::asset::PackageIndex index(vfs_shared_ptr->OpenDirectory(""));
        std::string path = index["planets"].path;
        ASSERT_EQ(index["planets"].type, cqsp::asset::AssetType::HJSON);

        planets_hjson = LoadHjsonAsset(vfs_shared_ptr, path);
    }

    static void TearDownTestSuite() {}

    void SetUp() override {
        // TODO(EhWhoAmI): If we want further speedup we might be able to move this
        // into the
        cqsp::common::loading::PlanetLoader loader(game.GetUniverse());
        loader.LoadHjson(planets_hjson);

        // 1 tick to initialize the universe
        Tick(1);
    }

    void TearDown() override {}

    static Hjson::Value planets_hjson;

    cqsp::common::Game game;
    cqsp::common::Universe& universe;
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

    SysOrbitTest() : game(), universe(game.GetUniverse()), simulation(game) { simulation.CreateSystems(); }
};

Hjson::Value SysOrbitTest::planets_hjson;

TEST_F(SysOrbitTest, BasicOrbitTest) {
    // Add something to orbit
    entt::entity earth = universe.planets["earth"];
    // Let's add something into orbit
    // Let's set this to LEO, at 500 km
    auto& body_component = universe.get<cqsp::common::components::bodies::Body>(earth);

    entt::entity ship1 = cqsp::common::actions::LaunchShip(
        game.GetUniverse(),
        cqsp::common::components::types::Orbit(body_component.radius + 500., 0.00001, 0, 0.01, 0.1, 0, earth));

    Tick(10000);
}

class PlaneMatchTests : public SysOrbitTest,
                        public testing::WithParamInterface<
                            std::pair<cqsp::common::components::types::Orbit, cqsp::common::components::types::Orbit>> {
};

INSTANTIATE_TEST_SUITE_P(
    PlaneMatchingTests, PlaneMatchTests,
    testing::Values(std::make_pair(cqsp::common::components::types::Orbit(6371 + 500., 0.00001, 0, 0, 0.1, 0),
                                   cqsp::common::components::types::Orbit(6371 + 500., 0.00001, 0.2, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(10000, 0.00001, 0, 0, 0.1, 0),
                                   cqsp::common::components::types::Orbit(10000, 0.00001, 0.2, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 0.5, 1.5, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 4.5, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 0, 0.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 5.4, 5.8, 0.1, 0)),
                    std::make_pair(cqsp::common::components::types::Orbit(50000, 0.00001, 5.4, 5.8, 0.1, 0),
                                   cqsp::common::components::types::Orbit(50000, 0.00001, 0.2, 0.8, 0.1, 0))));

TEST_P(PlaneMatchTests, MatchPlaneTest) {
    // Add something to orbit
    entt::entity earth = universe.planets["earth"];
    // Let's add something into orbit
    // Let's set this to LEO, at 500 km
    auto& earth_body_component = universe.get<cqsp::common::components::bodies::Body>(earth);
    // Chaser ship
    cqsp::common::components::types::Orbit source_orbit = GetParam().first;
    source_orbit.GM = earth_body_component.GM;
    source_orbit.reference_body = earth;
    entt::entity ship1 = cqsp::common::actions::LaunchShip(game.GetUniverse(), source_orbit);

    cqsp::common::components::types::Orbit target_orbit = GetParam().second;
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

    auto& kinematics1 = universe.get<cqsp::common::components::types::Kinematics>(ship1);
    auto& kinematics2 = universe.get<cqsp::common::components::types::Kinematics>(ship2);
    glm::dvec3 angular_momentum1 = glm::cross(kinematics1.position, kinematics1.velocity);
    glm::dvec3 angular_momentum2 = glm::cross(kinematics2.position, kinematics2.velocity);
    double angle = glm::angle(glm::normalize(angular_momentum1), glm::normalize(angular_momentum2));
    EXPECT_NEAR(angle, 0, 0.00001);
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
        entt::entity earth = universe.planets["earth"];
        // Let's add something into orbit
        // Let's set this to LEO, at 500 km
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

    TickSeconds(circularize.second + 1.);

    // Now check the new periapsis is the same as the new apoapsis
    double initial_apoapsis = source_orbit.GetApoapsis();
    EXPECT_NEAR(initial_apoapsis, ship_orbit.GetPeriapsis(), 1.);

    EXPECT_NO_FATAL_FAILURE({ ExpectNoPlaneChange(ship); });
    // Now verify that the amount of delta v we put into it is also the amount we would expect
    // This can be more inaccurate because we are sourcing these values from many different calculations, so we
    // must allow for some numerical inaccuricies
    EXPECT_NEAR(
        ship_orbit.OrbitalVelocityAtTrueAnomaly(0),
        glm::length(circularize.first) + source_orbit.OrbitalVelocityAtTrueAnomaly(cqsp::common::components::types::PI),
        2);

    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(0),
                cqsp::common::components::types::GetCircularOrbitingVelocity(ship_orbit.GM, ship_orbit.semi_major_axis),
                1);
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

    TickSeconds(circularize.second + 1.);

    // Now check the new periapsis is the same as the new apoapsis
    double initial_periapsis = source_orbit.GetPeriapsis();
    EXPECT_NEAR(initial_periapsis, ship_orbit.GetPeriapsis(), 1.);

    EXPECT_NO_FATAL_FAILURE({ ExpectNoPlaneChange(ship); });

    // Now verify that the amount of delta v we put into it is also the amount we would expect
    // This can be more inaccurate because we are sourcing these values from many different calculations, so we
    // must allow for some numerical inaccuricies
    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(cqsp::common::components::types::PI),
                // Subtract as periapsis operation is reducing orbital energy
                source_orbit.OrbitalVelocityAtTrueAnomaly(0) - glm::length(circularize.first), 2);

    EXPECT_NEAR(ship_orbit.OrbitalVelocityAtTrueAnomaly(cqsp::common::components::types::PI),
                cqsp::common::components::types::GetCircularOrbitingVelocity(ship_orbit.GM, ship_orbit.semi_major_axis),
                1);
}

// We shouldn't do a lot of orbits that have no inclination because if we have no inclination we don't really have a difference with
// our LAN.
INSTANTIATE_TEST_SUITE_P(BasicCircularizeTest, CircularizeTests,
                         testing::Values(cqsp::common::components::types::Orbit(10000., 0.2, 0, 0, 0.1, 0),
                                         cqsp::common::components::types::Orbit(10000., 0.3, 0.3, 0, 0.1, 0),
                                         cqsp::common::components::types::Orbit(10000., 0.3, 0.1, 0.3, 0.1, 0),
                                         cqsp::common::components::types::Orbit(10000., 0.3, 0.1, 0.3, 0.1, 1.2),
                                         cqsp::common::components::types::Orbit(20000., 0.6, 0.1, 0.3, 0.1, 1.2)));
