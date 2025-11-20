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

#include "client/scenes/universe/views/starsystemcamera.h"
#include "client/scenes/universe/views/starsystemcontroller.h"
#include "core/universe.h"

namespace cqsp::client::systems {
class SysStarSystemRenderer;
class StarSystemViewUI {
 public:
    StarSystemViewUI(core::Universe&, SysStarSystemRenderer&, StarSystemController&, StarSystemCamera&);
    void RenderInformationWindow(double delta_time);
    void RenderSelectedObjectInformation();

    void DoUI(float delta_time);

 private:
    StarSystemController& controller;
    StarSystemCamera& camera;
    core::Universe& universe;
    SysStarSystemRenderer& renderer;
};
};  // namespace cqsp::client::systems