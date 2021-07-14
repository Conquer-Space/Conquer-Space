/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <sol/sol.hpp>

namespace conquerspace {
namespace common {
namespace event {
struct EventResult {
    std::string name;
    std::string tooltip;
    sol::function action;
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
}  // namespace conquerspace
