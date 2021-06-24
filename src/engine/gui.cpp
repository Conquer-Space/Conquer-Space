/*
* Copyright 2021 Conquer Space
*/
#include "engine/gui.h"

#include <implot_internal.h>

namespace ImPlot {
template IMPLOT_API void PlotPieChart<float>(std::map<std::string, float>& map,
                                             double x, double y, double radius,
                                             bool normalize, const char* fmt,
                                             double angle0);
template IMPLOT_API void PlotPieChart<double>(std::map<std::string, double>& map,
                                              double x, double y, double radius,
                                              bool normalize, const char* fmt,
                                              double angle0);
template IMPLOT_API void PlotPieChart<int>(std::map<std::string, int>& map,
                                              double x, double y, double radius,
                                              bool normalize, const char* fmt,
                                              double angle0);

inline void RenderPieSlice(ImDrawList& DrawList, const ImPlotPoint& center,
                           double radius, double a0, double a1, ImU32 col) {
    static const float resolution = 50 / (2 * IM_PI);
    static ImVec2 buffer[50];
    buffer[0] = PlotToPixels(center);
    int n = ImMax(3, static_cast<int>((a1 - a0) * resolution));
    double da = (a1 - a0) / (n - 1);
    for (int i = 0; i < n; ++i) {
        double a = a0 + i * da;
        buffer[i + 1] = PlotToPixels(center.x + radius * cos(a), center.y + radius * sin(a));
    }
    DrawList.AddConvexPolyFilled(buffer, n + 1, col);
}

template <typename T>
IMPLOT_API void PlotPieChart(std::map<std::string, T>& map, double x,
                                     double y, double radius, bool normalize,
                                     const char* fmt, double angle0) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL,
        "PlotPieChart() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList& DrawList = *GetPlotDrawList();
    double sum = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        sum += static_cast<double>(it->second);
    }

    normalize = normalize || sum > 1.0;
    ImPlotPoint center(x, y);
    PushPlotClipRect();
    double a0 = angle0 * 2 * IM_PI / 360.0;
    double a1 = angle0 * 2 * IM_PI / 360.0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        double percent = normalize ? static_cast<double>(it->second) / sum :
                                                        static_cast<double>(it->second);
        a1 = a0 + 2 * IM_PI * percent;
        if (BeginItem(it->first.c_str())) {
            if (FitThisFrame()) {
                FitPoint(ImPlotPoint(x - radius, y - radius));
                FitPoint(ImPlotPoint(x + radius, y + radius));
            }
            ImU32 col = GetCurrentItem()->Color;
            if (percent < 0.5) {
                RenderPieSlice(DrawList, center, radius, a0, a1, col);
            } else {
                RenderPieSlice(DrawList, center, radius, a0,
                               a0 + (a1 - a0) * 0.5, col);
                RenderPieSlice(DrawList, center, radius, a0 + (a1 - a0) * 0.5,
                               a1, col);
            }
            EndItem();
        }
        a0 = a1;
    }
    if (fmt != NULL) {
        a0 = angle0 * 2 * IM_PI / 360.0;
        a1 = angle0 * 2 * IM_PI / 360.0;
        char buffer[32];
        for (auto it = map.begin(); it != map.end(); ++it) {
            ImPlotItem* item = GetItem(it->first.c_str());
            double percent =
                normalize ? static_cast<double>(it->second) / sum :
                                    static_cast<double>(it->second);
            a1 = a0 + 2 * IM_PI * percent;
            if (item->Show) {
                snprintf(buffer, sizeof(buffer), fmt, static_cast<double>(it->second));
                ImVec2 size = ImGui::CalcTextSize(buffer);
                double angle = a0 + (a1 - a0) * 0.5;
                ImVec2 pos = PlotToPixels(center.x + 0.5 * radius * cos(angle),
                                          center.y + 0.5 * radius * sin(angle));
                ImU32 col =
                    CalcTextColor(ImGui::ColorConvertU32ToFloat4(item->Color));
                DrawList.AddText(pos - size * 0.5f, col, buffer);
            }
            a0 = a1;
        }
    }
    PopPlotClipRect();
}
}  // namespace ImPlot
