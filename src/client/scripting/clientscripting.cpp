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
#include "client/scripting/clientscripting.h"

#include <imgui.h>

#include <map>
#include <string>
#include <vector>

#include "common/scripting/functionreg.h"

namespace cqsp::client::scripting {
namespace {
sol::object JsonToLuaObject(const Hjson::Value& j, const sol::this_state& s) {
    sol::state_view lua(s);
    switch (j.type()) {
        case Hjson::Type::String:
            return sol::make_object(lua, j.to_string());
        case Hjson::Type::Bool:
            return sol::make_object(lua, j.operator bool());
        case Hjson::Type::Double:
            return sol::make_object(lua, j.to_double());
        case Hjson::Type::Int64:
            return sol::make_object(lua, j.to_int64());
        case Hjson::Type::Map: {
            std::map<std::string, sol::object> obj;
            for (auto it = j.begin(); it != j.end(); ++it) {
                sol::object ob = JsonToLuaObject(it->second, s);
                obj[it->first] = ob;
            }
            return sol::make_object(lua, obj);
        }
        case Hjson::Type::Vector: {
            std::vector<sol::object> vec;
            vec.reserve(static_cast<int>(j.size()));
            for (auto index = 0; index < static_cast<int>(j.size()); ++index) {
                vec.push_back(JsonToLuaObject(j[index], s));
            }
            return sol::make_object(lua, vec);
        }
        default:
        case Hjson::Type::Null:
            return sol::make_object(lua, sol::nil);
    }
}

void AssetManagerInterfaces(asset::AssetManager& asset_manager, common::Universe& universe,
                            common::scripting::ScriptInterface& script_engine) {
    CREATE_NAMESPACE(client);

    script_engine.set_function("require", [&](const char* script) {
        using asset::TextDirectoryAsset;
        // Get script from asset loader
        asset::TextDirectoryAsset* asset = asset_manager.GetAsset<TextDirectoryAsset>("core:scripts");
        // Get the thing
        if (asset->paths.find(script) != asset->paths.end()) {
            return script_engine.require_script(script, asset->paths[script].data);
        } else {
            SPDLOG_INFO("Cannot find require {}", script);
            return sol::make_object(script_engine, sol::nil);
        }
    });

    REGISTER_FUNCTION("get_text_asset", [&](const char* id) {
        cqsp::asset::TextAsset* asset = asset_manager.GetAsset<cqsp::asset::TextAsset>(id);
        return sol::make_object<std::string>(script_engine, asset->data);
    });

    REGISTER_FUNCTION("get_hjson_asset", [&](const char* string, sol::this_state s) -> sol::table {
        cqsp::asset::HjsonAsset* as = asset_manager.GetAsset<cqsp::asset::HjsonAsset>(string);
        // Create json object.
        return JsonToLuaObject(as->data, s).as<sol::table>();
    });
}
}  // namespace

void ClientFunctions(asset::AssetManager& asset_manager, common::Universe& universe,
                     common::scripting::ScriptInterface& script_engine) {
    AssetManagerInterfaces(asset_manager, universe, script_engine);
}
};  // namespace cqsp::client::scripting
