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

#include "engine/application.h"

namespace cqsp {
namespace client {
namespace systems {
class SysUserInterface {
 public:
    explicit SysUserInterface(cqsp::engine::Application &_app) : m_app(_app) {}
    virtual void Init() = 0;
    virtual void DoUI(int delta_time) = 0;
    virtual void DoUpdate(int delta_time) = 0;

    cqsp::engine::Application &GetApp() { return m_app; }
    cqsp::common::Universe &GetUniverse() { return GetApp().GetUniverse(); }
    cqsp::asset::AssetManager &GetAssetManager() {
        return GetApp().GetAssetManager();
    }
    ImGuiWindowFlags window_flags = 0;

 private:
    cqsp::engine::Application &m_app;
};

class SysRmlUiInterface {
 public:
    explicit SysRmlUiInterface(cqsp::engine::Application& app) : m_app(app) {}
    virtual ~SysRmlUiInterface() {}
    virtual void Update(double delta_time) = 0;
    virtual void OpenDocument() = 0;

 protected:
    cqsp::engine::Application &GetApp() { return m_app; }
    cqsp::common::Universe &GetUniverse() { return GetApp().GetUniverse(); }
    cqsp::asset::AssetManager &GetAssetManager() {
        return GetApp().GetAssetManager();
    }

 private:
    cqsp::engine::Application &m_app;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
