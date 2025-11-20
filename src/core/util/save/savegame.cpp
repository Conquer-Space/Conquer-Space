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
#include "core/util/save/savegame.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "core/components/name.h"
#include "core/components/player.h"
#include "core/util/paths.h"
#include "core/util/save/save.h"
namespace cqsp::core::save {

void save_game(Universe& universe) {
    std::string save_dir_path = util::GetCqspSavePath();
    if (!std::filesystem::exists(save_dir_path)) std::filesystem::create_directories(save_dir_path);

    // Generate basic information
    Save save(universe);
    Hjson::Value metadata = save.GetMetadata();
    // Write to a file
    entt::entity player = universe.view<components::Player>().front();
    auto& name = universe.get<components::Identifier>(player);
    // Generate the folder
    std::filesystem::path path = std::filesystem::path(save_dir_path) / (name.identifier + "_" + universe.uuid);
    std::filesystem::create_directories(path);

    // Generate the file
    Hjson::MarshalToFile(save.GetMetadata(), GetMetaPath(path.string()));

    // Then write other game information, however we do that.
}

void load_game(Universe& universe, std::string_view directory) {
    Load load(universe);
    // Load meta file
    Hjson::Value metadata = Hjson::UnmarshalFromFile(GetMetaPath(directory));
    load.LoadMetadata(metadata);
}
}  // namespace cqsp::core::save
