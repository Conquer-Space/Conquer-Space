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

#include "client/systems/sysgui.h"

namespace cqsp::client::systems {
class SysOrbitFilter : public SysUserInterface {
 public:
    explicit SysOrbitFilter(engine::Application& app) : SysUserInterface(app) {}
    void Init() override;
    void DoUI(int delta_time) override;
    void DoUpdate(int delta_time) override;

 private:
    bool visible = true;
    bool hide_all_orbits = false;
};
}  // namespace cqsp::client::systems
