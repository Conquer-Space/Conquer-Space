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

#include "client/systems/sysgui.h"
#include "common/components/event.h"

namespace cqsp {
namespace client {
namespace systems {
namespace gui {
class SysEvent : public SysUserInterface {
 public:
    explicit SysEvent(cqsp::engine::Application& app)
        : SysUserInterface(app) {}

    void Init() override;
    void DoUI(float delta_time) override;
    void DoUpdate(float delta_time) override;
    void ProcessActionResult(const std::shared_ptr<cqsp::common::event::Event>&,
                             cqsp::common::event::EventQueue& queue);
    bool to_show;
};
}  // namespace gui
}  // namespace systems
}  // namespace client
}  // namespace cqsp
