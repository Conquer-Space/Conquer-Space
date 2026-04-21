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

#include <hjson.h>

#include <map>
#include <string_view>

#include "core/universe.h"

namespace cqsp::core::loading {
class HjsonLoader {
 public:
    explicit HjsonLoader(Universe& universe) : universe(universe) {}
    virtual ~HjsonLoader() = default;
    virtual const Hjson::Value& GetDefaultValues() = 0;
    int LoadHjson(const Hjson::Value& values);
    virtual bool LoadValue(const Hjson::Value& values, Node& node) = 0;
    virtual void PostLoad(const Node& node) {}
    virtual bool NeedIdentifier() { return true; }

 protected:
    Universe& universe;
};

class TagLoader {
 public:
    using Handler = std::function<void(Node&)>;
    void Register(std::string_view tag, Handler handler) { handles.emplace(std::string(tag), handler); }

    /**
      * Helper function to just emplace a component
      */
    template <typename Comp>
    void Register(std::string_view tag) {
        handles.emplace(std::string(tag), [](Node& node) { node.emplace<Comp>(); });
    }

    void Apply(std::string_view tag, Node& node) const;

    void ParseTags(const Hjson::Value& tags, Node& node) const;

 private:
    std::map<std::string, Handler> handles;
};
}  // namespace cqsp::core::loading
