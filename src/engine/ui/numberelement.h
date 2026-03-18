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

#include <iostream>

#include "RmlUi/Core/Element.h"
#include "RmlUi/Core/ElementText.h"
#include "engine/ui/DataExpression.h"
#include "engine/ui/DataView.h"

namespace cqsp::engine::ui {
class DataViewNumber final : public Rml::DataViewCommon {
 public:
    explicit DataViewNumber(Rml::Element* element);

    // Update the data view.
    // Returns true if the update resulted in a document change.
    bool Update(Rml::DataModel& model);

 private:
    Rml::String previous_rml;
};
}  // namespace cqsp::engine::ui
