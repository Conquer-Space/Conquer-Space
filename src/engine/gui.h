/*
* Copyright 2021 Conquer Space
*/
#pragma once

#define IMGUI_USER_CONFIG "engine/imconfig.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_markdown.h>

#include <implot.h>

#include <map>
#include <string>

namespace ImPlot {
template <typename T>
IMPLOT_API void PlotPieChart(std::map<std::string, T>&, double x, double y,
                             double radius, bool normalize = false,
                             const char* label_fmt = "%.1f",
                             double angle0 = 90);
}  // namespace ImPlot
