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
#include "engine/gui.h"

# define IMGUI_DEFINE_MATH_OPERATORS
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

void DrawFlowIcon(ImDrawList* drawList, const ImVec2& a,
                           const ImVec2& b, ax::Drawing::IconType type, bool filled,
                           ImU32 color, ImU32 innerColor) {
    auto rect           = ImRect(a, b);
    auto rect_w         = rect.Max.x - rect.Min.x;

    const auto origin_scale = rect_w / 24.0f;
    const auto outline_scale  = rect_w / 24.0f;

    const auto offset_x  = 1.0f * origin_scale;
    const auto offset_y  = 0.0f * origin_scale;
    const auto margin     = (filled ? 2.0f : 2.0f) * origin_scale;
    const auto rounding   = 0.1f * origin_scale;
    const auto tip_round  = 0.7f; // percentage of triangle edge (for tip)
    //const auto edge_round = 0.7f; // percentage of triangle edge (for corner)
    const auto canvas = ImRect(
        rect.Min.x + margin + offset_x,
        rect.Min.y + margin + offset_y,
        rect.Max.x - margin + offset_x,
        rect.Max.y - margin + offset_y);
    const auto canvas_x = canvas.Min.x;
    const auto canvas_y = canvas.Min.y;
    const auto canvas_w = canvas.Max.x - canvas.Min.x;
    const auto canvas_h = canvas.Max.y - canvas.Min.y;

    const auto left   = canvas_x + canvas_w            * 0.5f * 0.3f;
    const auto right  = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
    const auto top    = canvas_y + canvas_h            * 0.5f * 0.2f;
    const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
    const auto center_y = (top + bottom) * 0.5f;
    //const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

    const auto tip_top    = ImVec2(canvas_x + canvas_w * 0.5f, top);
    const auto tip_right  = ImVec2(right, center_y);
    const auto tip_bottom = ImVec2(canvas_x + canvas_w * 0.5f, bottom);

    drawList->PathLineTo(ImVec2(left, top) + ImVec2(0, rounding));
    drawList->PathBezierCurveTo(
        ImVec2(left, top),
        ImVec2(left, top),
        ImVec2(left, top) + ImVec2(rounding, 0));
    drawList->PathLineTo(tip_top);
    drawList->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
    drawList->PathBezierCurveTo(
        tip_right,
        tip_right,
        tip_bottom + (tip_right - tip_bottom) * tip_round);
    drawList->PathLineTo(tip_bottom);
    drawList->PathLineTo(ImVec2(left, bottom) + ImVec2(rounding, 0));
    drawList->PathBezierCurveTo(
        ImVec2(left, bottom),
        ImVec2(left, bottom),
        ImVec2(left, bottom) - ImVec2(0, rounding));

    if (!filled) {
        if (innerColor & 0xFF000000) {
            drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);
        }

        drawList->PathStroke(color, true, 2.0f * outline_scale);
    } else {
        drawList->PathFillConvex(color);
    }
}

void ax::Drawing::DrawIcon(ImDrawList* drawList, const ImVec2& a,
                           const ImVec2& b, IconType type, bool filled,
                           ImU32 color, ImU32 innerColor) {
    auto rect           = ImRect(a, b);
    auto rect_x         = rect.Min.x;
    auto rect_y         = rect.Min.y;
    auto rect_w         = rect.Max.x - rect.Min.x;
    auto rect_h         = rect.Max.y - rect.Min.y;
    auto rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

    if (type == IconType::Flow) {
        DrawFlowIcon(drawList, a, b, type, filled, color, innerColor);
    } else {
        auto triangleStart = rect_center_x + 0.32f * rect_w;

        auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

        rect.Min.x    += rect_offset;
        rect.Max.x    += rect_offset;
        rect_x        += rect_offset;
        rect_center_x += rect_offset * 0.5f;
        rect_center.x += rect_offset * 0.5f;

        if (type == IconType::Circle) {
            const auto c = rect_center;

            if (!filled) {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;

                if (innerColor & 0xFF000000)
                    drawList->AddCircleFilled(c, r, innerColor, 12 + extra_segments);
                drawList->AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
            } else {
                drawList->AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
            }
        }
        if (type == IconType::Square) {
            if (filled) {
                const auto r  = 0.5f * rect_w / 2.0f;
                const auto p0 = rect_center - ImVec2(r, r);
                const auto p1 = rect_center + ImVec2(r, r);

                drawList->AddRectFilled(p0, p1, color, 0, 15 + extra_segments);
            } else {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                const auto p0 = rect_center - ImVec2(r, r);
                const auto p1 = rect_center + ImVec2(r, r);

                if (innerColor & 0xFF000000)
                    drawList->AddRectFilled(p0, p1, innerColor, 0, 15 + extra_segments);

                drawList->AddRect(p0, p1, color, 0, 15 + extra_segments, 2.0f * outline_scale);
            }
        }
        if (type == IconType::Grid) {
            const auto r = 0.5f * rect_w / 2.0f;
            const auto w = ceilf(r / 3.0f);

            const auto baseTl = ImVec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
            const auto baseBr = ImVec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

            auto tl = baseTl;
            auto br = baseBr;
            for (int i = 0; i < 3; ++i) {
                tl.x = baseTl.x;
                br.x = baseBr.x;
                drawList->AddRectFilled(tl, br, color);
                tl.x += w * 2;
                br.x += w * 2;
                if (i != 1 || filled) {
                    drawList->AddRectFilled(tl, br, color);
                }
                tl.x += w * 2;
                br.x += w * 2;
                drawList->AddRectFilled(tl, br, color);

                tl.y += w * 2;
                br.y += w * 2;
            }

            triangleStart = br.x + w + 1.0f / 24.0f * rect_w;
        }
        if (type == IconType::RoundSquare) {
            if (filled) {
                const auto r  = 0.5f * rect_w / 2.0f;
                const auto cr = r * 0.5f;
                const auto p0 = rect_center - ImVec2(r, r);
                const auto p1 = rect_center + ImVec2(r, r);

                drawList->AddRectFilled(p0, p1, color, cr, 15);
            } else {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                const auto cr = r * 0.5f;
                const auto p0 = rect_center - ImVec2(r, r);
                const auto p1 = rect_center + ImVec2(r, r);

                if (innerColor & 0xFF000000) {
                    drawList->AddRectFilled(p0, p1, innerColor, cr, 15);
                }

                drawList->AddRect(p0, p1, color, cr, 15, 2.0f * outline_scale);
            }
        } else if (type == IconType::Diamond) {
            if (filled) {
                const auto r = 0.607f * rect_w / 2.0f;
                const auto c = rect_center;

                drawList->PathLineTo(c + ImVec2(0, -r));
                drawList->PathLineTo(c + ImVec2(r, 0));
                drawList->PathLineTo(c + ImVec2(0, r));
                drawList->PathLineTo(c + ImVec2(-r, 0));
                drawList->PathFillConvex(color);
            } else {
                const auto r = 0.607f * rect_w / 2.0f - 0.5f;
                const auto c = rect_center;

                drawList->PathLineTo(c + ImVec2(0, -r));
                drawList->PathLineTo(c + ImVec2(r, 0));
                drawList->PathLineTo(c + ImVec2(0, r));
                drawList->PathLineTo(c + ImVec2(-r, 0));

                if (innerColor & 0xFF000000) {
                    drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);
                }

                drawList->PathStroke(color, true, 2.0f * outline_scale);
            }
        } else {
            const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

            drawList->AddTriangleFilled(
                ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
                color);
        }
    }
}
