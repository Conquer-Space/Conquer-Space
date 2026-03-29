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
#include "client/scripting/implotfunctions.h"

#include <implot.h>

#include <string>
#include <vector>

namespace cqsp::client::scripting {
namespace {
std::vector<double> TableToVector(const sol::table& t) {
    std::vector<double> v;
    v.reserve(t.size());
    for (size_t i = 1; i <= t.size(); i++) {
        v.push_back(t.get<double>(i));
    }
    return v;
}
}  // namespace

void LoadImPlotFunctions(core::Universe& universe, core::scripting::ScriptInterface& script_engine) {
    auto ImPlot = script_engine["ImPlot"].get_or_create<sol::table>();

    // Enums
    script_engine.new_enum("ImPlotFlags", "None", ImPlotFlags_None, "NoTitle", ImPlotFlags_NoTitle, "NoLegend",
                           ImPlotFlags_NoLegend, "NoMouseText", ImPlotFlags_NoMouseText, "NoInputs",
                           ImPlotFlags_NoInputs, "NoMenus", ImPlotFlags_NoMenus, "NoBoxSelect", ImPlotFlags_NoBoxSelect,
                           "NoChild", ImPlotFlags_NoChild, "NoFrame", ImPlotFlags_NoFrame, "Equal", ImPlotFlags_Equal,
                           "Crosshairs", ImPlotFlags_Crosshairs, "CanvasOnly", ImPlotFlags_CanvasOnly);

    script_engine.new_enum(
        "ImPlotAxisFlags", "None", ImPlotAxisFlags_None, "NoLabel", ImPlotAxisFlags_NoLabel, "NoGridLines",
        ImPlotAxisFlags_NoGridLines, "NoTickMarks", ImPlotAxisFlags_NoTickMarks, "NoTickLabels",
        ImPlotAxisFlags_NoTickLabels, "NoInitialFit", ImPlotAxisFlags_NoInitialFit, "NoMenus", ImPlotAxisFlags_NoMenus,
        "NoSideSwitch", ImPlotAxisFlags_NoSideSwitch, "NoHighlight", ImPlotAxisFlags_NoHighlight, "Opposite",
        ImPlotAxisFlags_Opposite, "Foreground", ImPlotAxisFlags_Foreground, "Invert", ImPlotAxisFlags_Invert, "AutoFit",
        ImPlotAxisFlags_AutoFit, "RangeFit", ImPlotAxisFlags_RangeFit, "PanStretch", ImPlotAxisFlags_PanStretch,
        "LockMin", ImPlotAxisFlags_LockMin, "LockMax", ImPlotAxisFlags_LockMax, "Lock", ImPlotAxisFlags_Lock,
        "NoDecorations", ImPlotAxisFlags_NoDecorations, "AuxDefault", ImPlotAxisFlags_AuxDefault);

    script_engine.new_enum("ImAxis", "X1", ImAxis_X1, "X2", ImAxis_X2, "X3", ImAxis_X3, "Y1", ImAxis_Y1, "Y2",
                           ImAxis_Y2, "Y3", ImAxis_Y3);

    script_engine.new_enum("ImPlotCond", "None", ImPlotCond_None, "Always", ImPlotCond_Always, "Once", ImPlotCond_Once);

    script_engine.new_enum("ImPlotLocation", "Center", ImPlotLocation_Center, "North", ImPlotLocation_North, "South",
                           ImPlotLocation_South, "West", ImPlotLocation_West, "East", ImPlotLocation_East, "NorthWest",
                           ImPlotLocation_NorthWest, "NorthEast", ImPlotLocation_NorthEast, "SouthWest",
                           ImPlotLocation_SouthWest, "SouthEast", ImPlotLocation_SouthEast);

    script_engine.new_enum("ImPlotLineFlags", "None", ImPlotLineFlags_None, "Segments", ImPlotLineFlags_Segments,
                           "Loop", ImPlotLineFlags_Loop, "SkipNaN", ImPlotLineFlags_SkipNaN, "NoClip",
                           ImPlotLineFlags_NoClip, "Shaded", ImPlotLineFlags_Shaded);

    script_engine.new_enum("ImPlotBarsFlags", "None", ImPlotBarsFlags_None, "Horizontal", ImPlotBarsFlags_Horizontal);

    script_engine.new_enum("ImPlotPieChartFlags", "None", ImPlotPieChartFlags_None, "Normalize",
                           ImPlotPieChartFlags_Normalize);

    script_engine.new_enum("ImPlotScatterFlags", "None", ImPlotScatterFlags_None, "NoClip", ImPlotScatterFlags_NoClip);

#pragma region Begin/End Plot
    ImPlot.set_function("BeginPlot",
                        sol::overload([](const std::string& title) { return ImPlot::BeginPlot(title.c_str()); },
                                      [](const std::string& title, float width, float height) {
                                          return ImPlot::BeginPlot(title.c_str(), ImVec2(width, height));
                                      },
                                      [](const std::string& title, float width, float height, int flags) {
                                          return ImPlot::BeginPlot(title.c_str(), ImVec2(width, height), flags);
                                      }));
    ImPlot.set_function("EndPlot", []() { ImPlot::EndPlot(); });
#pragma endregion Begin / End Plot

#pragma region Setup
    ImPlot.set_function(
        "SetupAxes",
        sol::overload([]() { ImPlot::SetupAxes(nullptr, nullptr); },
                      [](const std::string& x_label, const std::string& y_label) {
                          ImPlot::SetupAxes(x_label.c_str(), y_label.c_str());
                      },
                      [](const std::string& x_label, const std::string& y_label, int x_flags, int y_flags) {
                          ImPlot::SetupAxes(x_label.c_str(), y_label.c_str(), x_flags, y_flags);
                      },
                      [](sol::lua_nil_t, sol::lua_nil_t, int x_flags, int y_flags) {
                          ImPlot::SetupAxes(nullptr, nullptr, x_flags, y_flags);
                      }));

    ImPlot.set_function("SetupAxesLimits",
                        sol::overload([](double x_min, double x_max, double y_min,
                                         double y_max) { ImPlot::SetupAxesLimits(x_min, x_max, y_min, y_max); },
                                      [](double x_min, double x_max, double y_min, double y_max, int cond) {
                                          ImPlot::SetupAxesLimits(x_min, x_max, y_min, y_max, cond);
                                      }));

    ImPlot.set_function(
        "SetupAxis",
        sol::overload(
            [](int axis) { ImPlot::SetupAxis(axis); },
            [](int axis, const std::string& label) { ImPlot::SetupAxis(axis, label.c_str()); },
            [](int axis, const std::string& label, int flags) { ImPlot::SetupAxis(axis, label.c_str(), flags); },
            [](int axis, sol::lua_nil_t, int flags) { ImPlot::SetupAxis(axis, nullptr, flags); }));

    ImPlot.set_function(
        "SetupAxisLimits",
        sol::overload(
            [](int axis, double v_min, double v_max) { ImPlot::SetupAxisLimits(axis, v_min, v_max); },
            [](int axis, double v_min, double v_max, int cond) { ImPlot::SetupAxisLimits(axis, v_min, v_max, cond); }));

    ImPlot.set_function("SetupLegend",
                        sol::overload([](int location) { ImPlot::SetupLegend(location); },
                                      [](int location, int flags) { ImPlot::SetupLegend(location, flags); }));

    ImPlot.set_function("SetupFinish", []() { ImPlot::SetupFinish(); });
#pragma endregion Setup

#pragma region SetNext
    ImPlot.set_function("SetNextAxesToFit", []() { ImPlot::SetNextAxesToFit(); });
    ImPlot.set_function("SetNextAxisToFit", [](int axis) { ImPlot::SetNextAxisToFit(axis); });
    ImPlot.set_function("SetNextAxesLimits",
                        sol::overload([](double x_min, double x_max, double y_min,
                                         double y_max) { ImPlot::SetNextAxesLimits(x_min, x_max, y_min, y_max); },
                                      [](double x_min, double x_max, double y_min, double y_max, int cond) {
                                          ImPlot::SetNextAxesLimits(x_min, x_max, y_min, y_max, cond);
                                      }));
    ImPlot.set_function(
        "SetNextAxisLimits",
        sol::overload([](int axis, double v_min, double v_max) { ImPlot::SetNextAxisLimits(axis, v_min, v_max); },
                      [](int axis, double v_min, double v_max, int cond) {
                          ImPlot::SetNextAxisLimits(axis, v_min, v_max, cond);
                      }));
#pragma endregion SetNext

#pragma region Plot Items
    // PlotLine: y values only (auto x from index)
    // PlotLine: separate x and y vectors
    ImPlot.set_function(
        "PlotLine",
        sol::overload(
            [](const std::string& label, const std::vector<double>& ys) {
                ImPlot::PlotLine(label.c_str(), ys.data(), static_cast<int>(ys.size()));
            },
            [](const std::string& label, const std::vector<double>& xs, const std::vector<double>& ys) {
                int count = static_cast<int>(std::min(xs.size(), ys.size()));
                ImPlot::PlotLine(label.c_str(), xs.data(), ys.data(), count);
            },
            [](const std::string& label, const std::vector<double>& xs, const std::vector<double>& ys, int flags) {
                int count = static_cast<int>(std::min(xs.size(), ys.size()));
                ImPlot::PlotLine(label.c_str(), xs.data(), ys.data(), count, flags);
            }));

    // PlotBars: y values only
    // PlotBars: separate x and y vectors with bar_size
    ImPlot.set_function(
        "PlotBars", sol::overload(
                        [](const std::string& label, const std::vector<double>& ys) {
                            ImPlot::PlotBars(label.c_str(), ys.data(), static_cast<int>(ys.size()));
                        },
                        [](const std::string& label, const std::vector<double>& ys, double bar_size) {
                            ImPlot::PlotBars(label.c_str(), ys.data(), static_cast<int>(ys.size()), bar_size);
                        },
                        [](const std::string& label, const std::vector<double>& ys, double bar_size, double shift) {
                            ImPlot::PlotBars(label.c_str(), ys.data(), static_cast<int>(ys.size()), bar_size, shift);
                        },
                        [](const std::string& label, const std::vector<double>& xs, const std::vector<double>& ys,
                           double bar_size) {
                            int count = static_cast<int>(std::min(xs.size(), ys.size()));
                            ImPlot::PlotBars(label.c_str(), xs.data(), ys.data(), count, bar_size);
                        }));

    // PlotScatter: y values only
    // PlotScatter: separate x and y vectors
    ImPlot.set_function("PlotScatter",
                        sol::overload(
                            [](const std::string& label, const std::vector<double>& ys) {
                                ImPlot::PlotScatter(label.c_str(), ys.data(), static_cast<int>(ys.size()));
                            },
                            [](const std::string& label, const std::vector<double>& xs, const std::vector<double>& ys) {
                                int count = static_cast<int>(std::min(xs.size(), ys.size()));
                                ImPlot::PlotScatter(label.c_str(), xs.data(), ys.data(), count);
                            }));

    // PlotPieChart: labels as Lua table of strings, values as Lua table of numbers
    ImPlot.set_function(
        "PlotPieChart",
        sol::overload(
            [](const sol::table& label_table, const sol::table& value_table, double x, double y, double radius) {
                std::vector<std::string> label_strings;
                std::vector<const char*> labels;
                auto values = TableToVector(value_table);
                label_strings.reserve(label_table.size());
                for (size_t i = 1; i <= label_table.size(); i++) {
                    label_strings.push_back(label_table.get<std::string>(i));
                }
                labels.reserve(label_strings.size());
                for (const auto& s : label_strings) {
                    labels.push_back(s.c_str());
                }
                int count = static_cast<int>(std::min(labels.size(), values.size()));
                ImPlot::PlotPieChart(labels.data(), values.data(), count, x, y, radius);
            },
            [](const sol::table& label_table, const sol::table& value_table, double x, double y, double radius,
               const std::string& label_fmt) {
                std::vector<std::string> label_strings;
                std::vector<const char*> labels;
                auto values = TableToVector(value_table);
                label_strings.reserve(label_table.size());
                for (size_t i = 1; i <= label_table.size(); i++) {
                    label_strings.push_back(label_table.get<std::string>(i));
                }
                labels.reserve(label_strings.size());
                for (const auto& s : label_strings) {
                    labels.push_back(s.c_str());
                }
                int count = static_cast<int>(std::min(labels.size(), values.size()));
                ImPlot::PlotPieChart(labels.data(), values.data(), count, x, y, radius, label_fmt.c_str());
            },
            [](const sol::table& label_table, const sol::table& value_table, double x, double y, double radius,
               const std::string& label_fmt, double angle0, int flags) {
                std::vector<std::string> label_strings;
                std::vector<const char*> labels;
                auto values = TableToVector(value_table);
                label_strings.reserve(label_table.size());
                for (size_t i = 1; i <= label_table.size(); i++) {
                    label_strings.push_back(label_table.get<std::string>(i));
                }
                labels.reserve(label_strings.size());
                for (const auto& s : label_strings) {
                    labels.push_back(s.c_str());
                }
                int count = static_cast<int>(std::min(labels.size(), values.size()));
                ImPlot::PlotPieChart(labels.data(), values.data(), count, x, y, radius, label_fmt.c_str(), angle0,
                                     flags);
            }));

    ImPlot.set_function(
        "PlotText",
        sol::overload([](const std::string& text, double x, double y) { ImPlot::PlotText(text.c_str(), x, y); },
                      [](const std::string& text, double x, double y, float pix_x, float pix_y) {
                          ImPlot::PlotText(text.c_str(), x, y, ImVec2(pix_x, pix_y));
                      }));
#pragma endregion Plot Items

#pragma region Plot Utils
    ImPlot.set_function("SetAxis", [](int axis) { ImPlot::SetAxis(axis); });
    ImPlot.set_function("SetAxes", [](int x_axis, int y_axis) { ImPlot::SetAxes(x_axis, y_axis); });
#pragma endregion Plot Utils
}
}  // namespace cqsp::client::scripting
