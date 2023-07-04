/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <memory>
#include <string>

#include "client/systems/clientscripting.h"
#include "common/scripting/luafunctions.h"
#include "common/systems/loading/hjsonloader.h"
#include "common/systems/loading/loadcities.h"
#include "common/systems/loading/loadcountries.h"
#include "common/systems/loading/loadgoods.h"
#include "common/systems/loading/loadnames.h"
#include "common/systems/loading/loadplanets.h"
#include "common/systems/loading/loadprovinces.h"
#include "common/systems/loading/loadsatellites.h"
#include "common/systems/loading/timezoneloader.h"
#include "common/systems/science/fields.h"
#include "common/systems/science/technology.h"
#include "common/systems/sysuniversegenerator.h"

namespace {
void LoadResource(cqsp::engine::Application& app, const std::string& asset_name,
                  void (*func)(cqsp::common::Universe& universe, Hjson::Value& recipes)) {
    namespace cqspc = cqsp::common::components;
    for (auto it = app.GetAssetManager().GetPackageBegin(); it != app.GetAssetManager().GetPackageEnd(); it++) {
        if (!it->second->HasAsset(asset_name)) {
            continue;
        }
        cqsp::asset::HjsonAsset* good_assets = it->second->GetAsset<cqsp::asset::HjsonAsset>(asset_name);
        try {
            func(app.GetUniverse(), good_assets->data);
        } catch (std::runtime_error& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: {}", asset_name, error.what());
        } catch (Hjson::index_out_of_bounds&) {
        }
    }
}

template <class T>
void LoadResource(cqsp::engine::Application& app, std::string asset_name) {
    using cqsp::common::systems::loading::HjsonLoader;
    static_assert(std::is_base_of<HjsonLoader, T>::value, "Class is not child of");
    std::unique_ptr<HjsonLoader> ptr = std::make_unique<T>(app.GetUniverse());

    namespace cqspc = cqsp::common::components;
    for (auto it = app.GetAssetManager().GetPackageBegin(); it != app.GetAssetManager().GetPackageEnd(); it++) {
        if (!it->second->HasAsset(asset_name)) {
            continue;
        }
        cqsp::asset::HjsonAsset* good_assets = it->second->GetAsset<cqsp::asset::HjsonAsset>(asset_name);
        try {
            ptr->LoadHjson(good_assets->data);
        } catch (std::runtime_error& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: {}", asset_name, error.what());
        } catch (Hjson::index_out_of_bounds&) {
        }
    }
}
}  // namespace

namespace cqsp::client::systems {
void LoadAllResources(cqsp::engine::Application& app) {
    using namespace cqsp::common::systems::loading;  // NOLINT
    LoadResource<GoodLoader>(app, "goods");
    LoadResource<RecipeLoader>(app, "recipes");
    LoadResource<PlanetLoader>(app, "planets");
    LoadResource<TimezoneLoader>(app, "timezones");
    LoadResource<CountryLoader>(app, "countries");
    LoadProvinces(app.GetUniverse(), app.GetAssetManager().GetAsset<asset::TextAsset>("province_defs")->data);
    LoadResource<CityLoader>(app, "cities");
    LoadResource(app, "names", LoadNameLists);
    LoadResource(app, "tech_fields", common::systems::science::LoadFields);
    LoadResource(app, "tech_list", common::systems::science::LoadTechnologies);
    common::systems::loading::LoadSatellites(app.GetUniverse(),
                                             app.GetAssetManager().GetAsset<asset::TextAsset>("satellites")->data);

    // Initialize planet terrains
    asset::HjsonAsset* asset = app.GetAssetManager().GetAsset<asset::HjsonAsset>("core:terrain_colors");
    common::systems::loading::LoadTerrainData(app.GetUniverse(), asset->data);

    // Load scripts
    // Load lua functions
    cqsp::scripting::LoadFunctions(app.GetUniverse(), app.GetScriptInterface());
    scripting::ClientFunctions(app);

    // Load universe
    // Register data groups
    auto& script_interface = app.GetScriptInterface();
    script_interface.RegisterDataGroup("generators");
    script_interface.RegisterDataGroup("events");
}
}  // namespace cqsp::client::systems
