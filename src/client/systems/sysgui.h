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

#include "client/conquerspace.h"
#include "engine/application.h"

namespace cqsp::client::systems {
class SysUserInterface {
 public:
    explicit SysUserInterface(engine::Application &_app) : m_app(_app) {}
    virtual ~SysUserInterface() = default;
    virtual void Init() = 0;
    virtual void DoUI(int delta_time) = 0;
    virtual void DoUpdate(int delta_time) = 0;

    engine::Application &GetApp() { return m_app; }
    common::Universe &GetUniverse() { return dynamic_cast<ConquerSpace *>(GetApp().GetGame())->m_universe; }
    common::scripting::ScriptInterface &GetScriptInterface() {
        return dynamic_cast<ConquerSpace *>(GetApp().GetGame())->script_interface;
    }
    asset::AssetManager &GetAssetManager() { return GetApp().GetAssetManager(); }
    ImGuiWindowFlags window_flags = 0;

 private:
    engine::Application &m_app;
};

class SysRmlUiInterface {
 public:
    explicit SysRmlUiInterface(engine::Application &app) : m_app(app) {}
    virtual ~SysRmlUiInterface() {}
    virtual void Update(double delta_time) = 0;
    virtual void OpenDocument() = 0;

 protected:
    engine::Application &GetApp() { return m_app; }
    common::Universe &GetUniverse() { return dynamic_cast<ConquerSpace *>(GetApp().GetGame())->m_universe; }
    asset::AssetManager &GetAssetManager() { return GetApp().GetAssetManager(); }

 private:
    engine::Application &m_app;
};
}  // namespace cqsp::client::systems
