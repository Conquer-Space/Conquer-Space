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
#include <exception>

#include <glm/glm.hpp>

#include "core/universe.h"
#include <cstdint>

namespace cqsp::core::loading {
class LoadingException : public std::exception {
   std::string msg;
 public:
   LoadingException(const std::string& msg) : msg(msg) {}
   const char* what() const override {
      return msg.c_str();
   }
};

class HjsonLoader {
 public:
    explicit HjsonLoader(Universe& universe) : universe(universe) {}
    virtual ~HjsonLoader() = default;
    virtual const Hjson::Value& GetDefaultValues() = 0;
    int LoadHjson(const Hjson::Value& values);
    virtual bool LoadValue(const Hjson::Value& values, Node& node) = 0;
    virtual void PostLoad(const Node& node) {}
    virtual bool NeedIdentifier() { return true; }

    virtual void OnThrow() {}

    double LoadDouble(const Hjson::Value& value, const std::string& name, double default_value = 0.);
    double RequiredDouble(const Hjson::Value& value, const std::string& name);

    std::string LoadString(const Hjson::Value& value, const std::string& name, const std::string& default_value = "");
    std::string RequiredString(const Hjson::Value& value, const std::string& name);

    Hjson::Value LoadMap(const Hjson::Value& value, const std::string& name);

    const std::string& GetIdentifier(const Node& node);

 protected:
    Universe& universe;

    glm::vec3 LoadColor(const Hjson::Value& value, std::string_view identifier);
    glm::vec3 LoadColor(const Hjson::Value& value);
    uint32_t StringHash(std::string_view);
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
