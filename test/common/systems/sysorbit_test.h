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
#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <numbers>
#include <memory>

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
#include "common/systems/movement/sysorbit.h"

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
