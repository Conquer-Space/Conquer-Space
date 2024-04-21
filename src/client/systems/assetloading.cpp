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

namespace cqsp::client::systems {
void LoadResource(cqsp::engine::Application& app, cqsp::common::Universe& universe, const std::string& asset_name,
                  void (*func)(cqsp::common::Universe& universe, Hjson::Value& recipes)) {
    namespace cqspc = cqsp::common::components;
    for (const auto& it : app.GetAssetManager()) {
        if (!it.second->HasAsset(asset_name)) {
            continue;
        }
        cqsp::asset::HjsonAsset* good_assets = it.second->GetAsset<cqsp::asset::HjsonAsset>(asset_name);
        try {
            func(universe, good_assets->data);
        } catch (std::runtime_error& error) {
            SPDLOG_INFO("Failed to load hjson asset {}: {}", asset_name, error.what());
        } catch (Hjson::index_out_of_bounds&) {
        }
    }
}

template <class T>
void LoadResource(cqsp::engine::Application& app, cqsp::common::Universe& universe, const std::string& asset_name) {
    using cqsp::common::systems::loading::HjsonLoader;
    static_assert(std::is_base_of<HjsonLoader, T>::value, "Class is not child of");
    std::unique_ptr<HjsonLoader> ptr = std::make_unique<T>(universe);

    namespace cqspc = cqsp::common::components;
    for (const auto& it : app.GetAssetManager()) {
        if (!it.second->HasAsset(asset_name)) {
            continue;
        }
        cqsp::asset::HjsonAsset* good_assets = it.second->GetAsset<cqsp::asset::HjsonAsset>(asset_name);
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
void LoadAllResources(cqsp::engine::Application& app, ConquerSpace& conquer_space) {
    using namespace cqsp::common::systems::loading;  // NOLINT
    LoadResource<GoodLoader>(app, conquer_space.GetUniverse(), "goods");
    LoadResource<RecipeLoader>(app, conquer_space.GetUniverse(), "recipes");
    LoadResource<PlanetLoader>(app, conquer_space.GetUniverse(), "planets");
    LoadResource<TimezoneLoader>(app, conquer_space.GetUniverse(), "timezones");
    LoadResource<CountryLoader>(app, conquer_space.GetUniverse(), "countries");
    LoadProvinces(conquer_space.GetUniverse(), app.GetAssetManager().GetAsset<asset::TextAsset>("province_defs")->data);
    LoadResource<CityLoader>(app, conquer_space.GetUniverse(), "cities");
    LoadResource(app, conquer_space.m_universe, "names", LoadNameLists);
    LoadResource(app, conquer_space.m_universe, "tech_fields", common::systems::science::LoadFields);
    LoadResource(app, conquer_space.m_universe, "tech_list", common::systems::science::LoadTechnologies);
    common::systems::loading::LoadSatellites(conquer_space.GetUniverse(),
                                             app.GetAssetManager().GetAsset<asset::TextAsset>("satellites")->data);

    // Initialize planet terrains
    asset::HjsonAsset* asset = app.GetAssetManager().GetAsset<asset::HjsonAsset>("core:terrain_colors");
    common::systems::loading::LoadTerrainData(conquer_space.GetUniverse(), asset->data);

    // Load scripts
    // Load lua functions
    common::scripting::LoadFunctions(conquer_space.GetUniverse(), conquer_space.GetScriptInterface());
    scripting::ClientFunctions(app, conquer_space.GetUniverse(), conquer_space.GetScriptInterface());

    // Load universe
    // Register data groups
    auto& script_interface = conquer_space.GetScriptInterface();
    script_interface.RegisterDataGroup("generators");
    script_interface.RegisterDataGroup("events");
}
}  // namespace cqsp::client::systems
