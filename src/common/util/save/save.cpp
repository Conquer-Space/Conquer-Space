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
#include "common/util/save/save.h"

#include <hjson.h>

#include <filesystem>
#include <string>

#include "common/version.h"

Hjson::Value cqsp::common::save::Save::GetMetadata() {
    // This generates the basic information of the save
    Hjson::Value value;
    value["date"] = universe.date.GetDate();
    value["uuid"] = universe.uuid;
    value["version"] = CQSP_VERSION;
    return value;
}

Hjson::Value cqsp::common::save::Save::SaveGame() {
    // Save all the game information
    return Hjson::Value();
}

void cqsp::common::save::Load::LoadMetadata(Hjson::Value& data) {
    universe.date.SetDate((int)data["date"]);
    // Verify version, but screw that
    universe.uuid = data["uuid"].to_string();
}

std::string cqsp::common::save::GetMetaPath(std::string_view folder) {
    return (std::filesystem::path(folder) / "meta.hjson").string();
}
