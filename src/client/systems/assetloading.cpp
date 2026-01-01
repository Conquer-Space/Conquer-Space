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
#include "client/systems/assetloading.h"

#include <chrono>
#include <memory>
#include <string>

#include "client/scripting/clientscripting.h"
#include "client/scripting/clientuielements.h"
#include "client/scripting/imguifunctions.h"
#include "core/components/surface.h"
#include "core/loading/fields.h"
#include "core/loading/hjsonloader.h"
#include "core/loading/loadcities.h"
#include "core/loading/loadcountries.h"
#include "core/loading/loadgoods.h"
#include "core/loading/loadnames.h"
#include "core/loading/loadsatellites.h"
#include "core/loading/loadterrain.h"
#include "core/loading/planetloader.h"
#include "core/loading/provinceloader.h"
#include "core/loading/recipeloader.h"
#include "core/loading/technology.h"
#include "core/loading/timezoneloader.h"
#include "core/scripting/luafunctions.h"
#include "core/systems/sysuniversegenerator.h"
#include "engine/asset/assetmanager.h"

namespace cqsp::client::systems {

namespace loading = core::loading;
namespace components = core::components;
using asset::AssetManager;
using asset::HjsonAsset;
using core::Universe;
using loading::HjsonLoader;

void LoadResource(AssetManager& asset_manager, Universe& universe, const std::string& asset_name,
                  void (*func)(Universe& universe, Hjson::Value& recipes)) {
    for (const auto& it : asset_manager) {
        if (!it.second->HasAsset(asset_name)) {
            continue;
        }
        HjsonAsset* good_assets = it.second->GetAsset<HjsonAsset>(asset_name);
        try {
            func(universe, good_assets->data);
        } catch (std::runtime_error& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: {}", asset_name, error.what());
        } catch (Hjson::index_out_of_bounds& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: Index out of bounds: {}", asset_name, error.what());
        }
    }
}

template <class T>
void LoadResource(AssetManager& asset_manager, Universe& universe, const std::string& asset_name) {
    auto start = std::chrono::system_clock::now();
    static_assert(std::is_base_of_v<HjsonLoader, T>, "Class is not child of");
    std::unique_ptr<HjsonLoader> ptr = std::make_unique<T>(universe);
    //SPDLOG_INFO("Starting load {}", asset_name);

    for (const auto& it : asset_manager) {
        if (!it.second->HasAsset(asset_name)) {
            continue;
        }
        HjsonAsset* good_assets = it.second->GetAsset<HjsonAsset>(asset_name);
        try {
            ptr->LoadHjson(good_assets->data);
        } catch (std::runtime_error& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: {}", asset_name, error.what());
        } catch (Hjson::index_out_of_bounds& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: Index out of bounds: {}", asset_name, error.what());
        }
    }
    auto end = std::chrono::system_clock::now();
    SPDLOG_INFO("{} load took {} ms", asset_name,
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

void LoadAllResources(AssetManager& asset_manager, ConquerSpace& conquer_space) {
    LoadResource<loading::GoodLoader>(asset_manager, conquer_space.GetUniverse(), "goods");
    LoadResource<loading::RecipeLoader>(asset_manager, conquer_space.GetUniverse(), "recipes");
    LoadResource<loading::PlanetLoader>(asset_manager, conquer_space.GetUniverse(), "planets");
    LoadResource<loading::TimezoneLoader>(asset_manager, conquer_space.GetUniverse(), "timezones");
    LoadResource<loading::CountryLoader>(asset_manager, conquer_space.GetUniverse(), "countries");

    // LoadPlanetProvinces(asset_manager, conquer_space);
    LoadResource<loading::ProvinceLoader>(asset_manager, conquer_space.GetUniverse(), "provinces");
    LoadResource<loading::CityLoader>(asset_manager, conquer_space.GetUniverse(), "cities");
    LoadResource<loading::SatelliteLoader>(asset_manager, conquer_space.GetUniverse(), "satellites");

    LoadResource(asset_manager, conquer_space.m_universe, "names", loading::LoadNameLists);
    LoadResource(asset_manager, conquer_space.m_universe, "tech_fields", loading::LoadFields);
    LoadResource(asset_manager, conquer_space.m_universe, "tech_list", loading::LoadTechnologies);

    // Initialize planet terrains
    HjsonAsset* asset = asset_manager.GetAsset<HjsonAsset>("core:terrain_colors");
    loading::LoadTerrainData(conquer_space.GetUniverse(), asset->data);

    // Load scripts
    // Load lua functions
    core::scripting::LoadFunctions(conquer_space.GetUniverse(), conquer_space.GetScriptInterface());
    scripting::LoadImGuiFunctions(conquer_space.GetUniverse(), conquer_space.GetScriptInterface());
    scripting::ClientFunctions(asset_manager, conquer_space.GetUniverse(), conquer_space.GetScriptInterface());
    scripting::InitClientElements(conquer_space.GetUniverse(), conquer_space.GetScriptInterface());

    // Load universe
    // Register data groups
    auto& script_interface = conquer_space.GetScriptInterface();
    script_interface.RegisterDataGroup("generators");
    script_interface.RegisterDataGroup("events");
    script_interface.RegisterDataGroup("interfaces");
}
}  // namespace cqsp::client::systems
