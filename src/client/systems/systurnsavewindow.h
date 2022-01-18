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

#include <vector>

#include "client/systems/sysgui.h"
#include "engine/application.h"

namespace cqsp {
namespace client {
namespace systems {

class SysTurnSaveWindow : public SysUserInterface {
 public:
    explicit SysTurnSaveWindow(cqsp::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(float delta_time) override;
    void DoUpdate(float delta_time) override;

    void TogglePlayState();

 private:
    double last_tick = 0;
    bool to_tick = false;
    std::vector<int> tick_speeds{1000, 500, 333, 100, 50, 10, 1};
    int tick_speed = tick_speeds.size() / 2;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
