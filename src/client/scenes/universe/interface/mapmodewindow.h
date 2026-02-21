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

#include "client/components/clientctx.h"
#include "client/systems/sysgui.h"

namespace cqsp::client::systems {
class MapModeWindow : public SysUserInterface {
 public:
    explicit MapModeWindow(engine::Application& app) : SysUserInterface(app) {}
    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    inline void SetMapMode(ctx::MapMode map_mode);
    inline ctx::MapMode GetMapMode();

    void MapModeButton(const char* string, ctx::MapMode map_mode);
    void MapModeButton(const char* string, ctx::MapMode map_mode, ctx::MapMode map_mode2);
};
}  // namespace cqsp::client::systems
