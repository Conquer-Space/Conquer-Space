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

#include <string>
#include <vector>
#include <memory>

#include <sol/sol.hpp>

namespace cqsp {
namespace common {
namespace event {
struct EventResult {
    std::string name;
    std::string tooltip;
    sol::function action;
    bool has_event;
};

struct Event {
    std::string title;
    std::string content;
    std::string image;
    sol::table table;
    std::vector<std::shared_ptr<EventResult>> actions;
};
struct EventQueue {
    std::vector<std::shared_ptr<Event>> events;
};
}  // namespace event
}  // namespace common
}  // namespace cqsp
