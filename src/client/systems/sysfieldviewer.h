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

#include <entt/entt.hpp>

#include "client/systems/sysgui.h"
#include "engine/application.h"

namespace cqsp {
namespace client {
namespace systems {
class SysFieldViewer : public SysUserInterface {
 public:
    explicit SysFieldViewer(cqsp::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    void FieldInformationWindow();
    entt::entity selected_tech;
};

class SysFieldNodeViewer : public SysUserInterface {
 public:
    explicit SysFieldNodeViewer(cqsp::engine::Application& app)
        : SysUserInterface(app) {}

    void Init();
    void DoUI(int delta_time);
    void DoUpdate(int delta_time);

 private:
    std::string hjson_content;
};
}  // namespace systems
}  // namespace client
}  // namespace cqsp
