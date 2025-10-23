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
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "common/game.h"
#include "common/loading/hjsonloader.h"
#include "common/loading/planetloader.h"
#include "common/simulation.cpp"
#include "common/systems/movement/sysorbit.h"
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

TEST(ManeuverTest, MatchPlaneTest) {
    // Now generate a bunch of orbits and universe and figure out if we can match to them
    // Generate a sim
    cqsp::common::Game game;
    std::filesystem::path data_path(cqsp::common::util::GetCqspTestDataPath());
    std::shared_ptr<cqsp::asset::NativeFileSystem> vfs_shared_ptr = std::shared_ptr<cqsp::asset::NativeFileSystem>(
        new cqsp::asset::NativeFileSystem((data_path / "core").string()));
    // Initialize a few planets
    // Load the core package
    cqsp::asset::PackageIndex index(vfs_shared_ptr->OpenDirectory(""));
    ManeuverTestSimulation simulation(game);
    std::string path = index["planets"].path;
    ASSERT_EQ(index["planets"].type, cqsp::asset::AssetType::HJSON);

    Hjson::Value planets_hjson = LoadHjsonAsset(vfs_shared_ptr, path);
    cqsp::common::loading::PlanetLoader loader(game.GetUniverse());
    loader.LoadHjson(planets_hjson);
    // Add something to orbit
    entt::entity earth = game.GetUniverse().planets["earth"];
    // Let's add something into orbit
}
