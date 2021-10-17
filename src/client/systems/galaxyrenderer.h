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
#include "engine/application.h"
#include "engine/renderer/renderable.h"

namespace cqsp::client::systems {
class GalaxyRenderer {
   public:
    GalaxyRenderer(cqsp::common::Universe &, cqsp::engine::Application &);
    void Initialize();
    void OnTick();
    void Render(float deltaTime);
    void Update(float deltaTime);
    void DoUI(float deltaTime);

 private:
    cqsp::common::Universe &m_universe;
    cqsp::engine::Application &m_app;

    cqsp::engine::Renderable star_system;

};
}  // namespace cqsp::client::systems
