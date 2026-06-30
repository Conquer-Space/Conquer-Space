/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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
#include "core/components/modifier.h"

namespace cqsp::core::components {

std::string_view ModifierTargetToString(ModifierTarget t) {
    switch (t) {
#define X(name, str)           \
    case ModifierTarget::name: \
        return str;
        CQSP_MODIFIER_TARGET_LIST
#undef X
    }
    return "";
}

std::optional<ModifierTarget> ModifierTargetFromString(std::string_view s) {
#define X(name, str) \
    if (s == str) return ModifierTarget::name;
    CQSP_MODIFIER_TARGET_LIST
#undef X
    return std::nullopt;
}

}  // namespace cqsp::core::components
