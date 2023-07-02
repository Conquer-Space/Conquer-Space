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
#pragma once

#define IMGUI_USER_CONFIG "engine/imconfig.h"

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_node_editor.h>
#include <imgui_stdlib.h>
#include <implot.h>

#include <map>
#include <string>
#include <utility>

namespace ed = ax::NodeEditor;

namespace ImPlot {
template <typename T>
IMPLOT_API void PlotPieChart(std::map<std::string, T>&, double x, double y, double radius, bool normalize = false,
                             const char* label_fmt = "%.1f", double angle0 = 90);
}  // namespace ImPlot

namespace ImGui {
template <typename... Args>
IMGUI_API void TextFmt(fmt::format_string<Args...> fmt, Args&&... args) {
    auto s = fmt::format(fmt, std::forward<Args>(args)...);
    Text("%s", s.c_str());
}

template <typename... Args>
IMGUI_API void TextFmtColored(const ImVec4& color, fmt::format_string<Args...> fmt, Args&&... args) {
    auto s = fmt::format(fmt, std::forward<Args>(args)...);
    TextColored(color, "%s", s.c_str());
}

template <typename... Args>
IMGUI_API bool SelectableFmt(fmt::format_string<Args...> fmt, bool* p_selected, ImGuiSelectableFlags flags,
                             const ImVec2& size, Args&&... args) {
    auto s = fmt::format(fmt, std::forward<Args>(args)...);
    return Selectable(s.c_str(), p_selected, flags, size);
}

template <typename... Args>
IMGUI_API bool SelectableFmt(fmt::format_string<Args...> fmt, bool* p_selected, ImGuiSelectableFlags flags,
                             Args&&... args) {
    auto s = fmt::format(fmt, std::forward<Args>(args)...);
    return Selectable(s.c_str(), p_selected, flags, ImVec2(0, 0));
}

template <typename... Args>
IMGUI_API bool SelectableFmt(fmt::format_string<Args...> fmt, bool* p_selected, Args&&... args) {
    auto s = fmt::format(fmt, std::forward<Args>(args)...);
    return Selectable(s.c_str(), p_selected, 0, ImVec2(0, 0));
}
}  // namespace ImGui

namespace ax::Drawing {
enum class IconType : ImU32 { Flow, Circle, Square, Grid, RoundSquare, Diamond };

void DrawIcon(ImDrawList* drawList, const ImVec2& a, const ImVec2& b, IconType type, bool filled, ImU32 color,
              ImU32 innerColor);

void Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1),
          const ImVec4& innerColor = ImVec4(0, 0, 0, 0));
}  // namespace ax::Drawing
