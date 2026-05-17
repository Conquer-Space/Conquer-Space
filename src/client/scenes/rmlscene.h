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

#include <memory>
#include <vector>

#include "client/scenes/scene.h"
#include "client/systems/sysgui.h"

namespace cqsp::client::scene {
class RmlClientScene : public ClientScene {
 public:
    explicit RmlClientScene(engine::Application& app) : ClientScene(app) {}

    template <class T>
    void AddRmlUiSystem() {
        auto ui = std::make_unique<T>(GetApp());
        ui->OpenDocument();
        documents.push_back(std::move(ui));
    }

    void CheckUiReload();

 protected:
    std::vector<std::unique_ptr<client::systems::SysRmlUiInterface>> documents;
};
}  // namespace cqsp::client::scene
