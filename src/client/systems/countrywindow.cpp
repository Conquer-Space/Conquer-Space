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
#include "client/systems/countrywindow.h"

#include "client/components/clientctx.h"
#include "client/systems/gui/systooltips.h"

namespace cqsp::client::systems {
void SysCountryInformation::Init() {}

void SysCountryInformation::DoUI(int delta_time) {
    // Get selected country
    entt::entity v =
        GetUniverse().view<cqsp::client::ctx::SelectedCountry>().front();
    if (v == entt::null) {
        return;
    }
    ImGui::Begin("Country Information");
    // Then do all countries and compile gdp data
    ImGui::TextFmt("{}", gui::GetName(GetUniverse(), v));
    ImGui::End();
}

void SysCountryInformation::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems