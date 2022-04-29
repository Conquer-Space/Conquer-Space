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
#pragma once

#define IMGUI_USER_CONFIG "engine/imconfig.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_markdown.h>
#include <imgui_node_editor.h>

#include <fmt/format.h>

#include <implot.h>
#include <map>
#include <string>
#include <utility>

namespace ed = ax::NodeEditor;

namespace ImPlot {
template <typename T>
IMPLOT_API void PlotPieChart(std::map<std::string, T>&, double x, double y,
                             double radius, bool normalize = false,
                             const char* label_fmt = "%.1f",
                             double angle0 = 90);
}  // namespace ImPlot

namespace ImGui {
template<typename S, typename... Args>
IMGUI_API void TextFmt(S fmt, Args&&...args) {
    Text(fmt::format(fmt, std::forward<Args>(args)...).c_str());
}
}
