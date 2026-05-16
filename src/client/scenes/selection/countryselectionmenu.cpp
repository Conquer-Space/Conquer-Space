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
#include "client/scenes/selection/countryselectionmenu.h"

namespace cqsp::client::systems::rmlui {
CountrySelectionMenu::~CountrySelectionMenu() {}
void CountrySelectionMenu::Update(double delta_time) {}
void CountrySelectionMenu::OpenDocument() {
    document = GetApp().LoadDocument(file_name);
    document->Show();
}
void CountrySelectionMenu::ReloadWindow() {}

void CountrySelectionMenu::DispatchEvent(const Rml::String& event, const Rml::Dictionary& parameters) {
    document->DispatchEvent(event, parameters);
}
}  // namespace cqsp::client::systems::rmlui
