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
#include "engine/ui/numberelement.h"

#include "core/util/utilnumberdisplay.h"

namespace cqsp::engine::ui {
DataViewNumber::DataViewNumber(Rml::Element* element) : Rml::DataViewCommon(element) {}

bool DataViewNumber::Update(Rml::DataModel& model) {
    bool result = false;
    Rml::Variant variant;
    Rml::Element* element = GetElement();
    Rml::DataExpressionInterface expr_interface(&model, element);

    if (element && GetExpression().Run(expr_interface, variant)) {
        if (variant.GetType() == Rml::Variant::STRING) {
            Rml::String new_rml = variant.Get<Rml::String>();
            if (new_rml != previous_rml) {
                element->SetInnerRML(new_rml);
                previous_rml = std::move(new_rml);
                result = true;
            }
            return result;
        }

        double value = variant.Get<double>();
        // Now convert
        Rml::String new_rml = cqsp::util::NumberToHumanString<double>(value);
        if (new_rml != previous_rml) {
            element->SetInnerRML(new_rml);
            previous_rml = std::move(new_rml);
            result = true;
        }
    }
    return result;
}
}  // namespace cqsp::engine::ui
