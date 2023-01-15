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

#include "common/universe.h"

namespace cqsp::common::save {
class Save {
 public:
    // Get the savegame?
    explicit Save(Universe& universe) : universe(universe) {}
    Universe& universe;

    Hjson::Value GetMetadata();
    Hjson::Value SaveGame();
};

class Load {
 public:
    explicit Load(Universe& universe) : universe(universe) {}

    void LoadMetadata(Hjson::Value& data);

    Universe& universe;
};
}  // namespace cqsp::common::save
