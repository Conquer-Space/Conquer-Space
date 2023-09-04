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
#pragma once

#include "client/conquerspace.h"
#include "engine/scene.h"

namespace cqsp::client {
class Scene : public cqsp::engine::Scene {
 public:
    explicit Scene(cqsp::engine::Application &app) : engine::Scene(app) {}
    cqsp::common::Universe &GetUniverse() { return dynamic_cast<ConquerSpace *>(GetApp().GetGame())->m_universe; }
    scripting::ScriptInterface &GetScriptInterface() {
        return dynamic_cast<ConquerSpace *>(GetApp().GetGame())->script_interface;
    }
};
}  // namespace cqsp::client
