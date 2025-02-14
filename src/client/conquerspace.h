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

#include "common/game.h"
#include "engine/gamestate.h"

namespace cqsp::client {
class ConquerSpace : public engine::GameState {
 public:
    ConquerSpace() : m_universe(game.GetUniverse()), script_interface(game.GetScriptInterface()) {}
    ~ConquerSpace() = default;

    common::Universe& m_universe;
    scripting::ScriptInterface& script_interface;

    common::Universe& GetUniverse() { return m_universe; }
    scripting::ScriptInterface& GetScriptInterface() { return script_interface; }

    common::Game& GetGame() { return game; }

 private:
    common::Game game;
};
}  // namespace cqsp::client
