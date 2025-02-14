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
#include "client/scripting/imguifunctions.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <string>

#include "client/scripting/imguienums.h"
#include "client/scripting/imguifunctiondefs.h"

namespace cqsp::client::scripting {
void LoadImGuiFunctions(cqsp::common::Universe& universe, cqsp::scripting::ScriptInterface& script_engine) {
    InitEnums(script_engine);
    auto ImGui = script_engine["ImGui"].get_or_create<sol::table>();

#pragma region Windows
    ImGui.set_function("Begin",
                       sol::overload(sol::resolve<bool(const std::string&)>(Begin),
                                     sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(Begin),
                                     sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int)>(Begin)));
    ImGui.set_function("End", End);
#pragma endregion Windows

#pragma region Child Windows
    ImGui.set_function("BeginChild",
                       sol::overload(sol::resolve<bool(const std::string&)>(BeginChild),
                                     sol::resolve<bool(const std::string&, float)>(BeginChild),
                                     sol::resolve<bool(const std::string&, float, float)>(BeginChild),
                                     sol::resolve<bool(const std::string&, float, float, bool)>(BeginChild),
                                     sol::resolve<bool(const std::string&, float, float, bool, int)>(BeginChild)));
    ImGui.set_function("EndChild", EndChild);
#pragma endregion Child Windows

#pragma region Window Utilities
    ImGui.set_function("IsWindowAppearing", IsWindowAppearing);
    ImGui.set_function("IsWindowCollapsed", IsWindowCollapsed);
    ImGui.set_function("IsWindowFocused",
                       sol::overload(sol::resolve<bool()>(IsWindowFocused), sol::resolve<bool(int)>(IsWindowFocused)));
    ImGui.set_function("IsWindowHovered",
                       sol::overload(sol::resolve<bool()>(IsWindowHovered), sol::resolve<bool(int)>(IsWindowHovered)));
    ImGui.set_function("GetWindowDpiScale", GetWindowDpiScale);
    ImGui.set_function("GetWindowPos", GetWindowPos);
    ImGui.set_function("GetWindowSize", GetWindowSize);
    ImGui.set_function("GetWindowWidth", GetWindowWidth);
    ImGui.set_function("GetWindowHeight", GetWindowHeight);

    ImGui.set_function("GetDisplaySize", GetDisplaySize);
    // Prefer  SetNext...
    ImGui.set_function("SetNextWindowPos",
                       sol::overload(sol::resolve<void(float, float)>(SetNextWindowPos),
                                     sol::resolve<void(float, float, int)>(SetNextWindowPos),
                                     sol::resolve<void(float, float, int, float, float)>(SetNextWindowPos)));
    ImGui.set_function("SetNextWindowSize", sol::overload(sol::resolve<void(float, float)>(SetNextWindowSize),
                                                          sol::resolve<void(float, float, int)>(SetNextWindowSize)));
    ImGui.set_function("SetNextWindowSizeConstraints", SetNextWindowSizeConstraints);
    ImGui.set_function("SetNextWindowContentSize", SetNextWindowContentSize);
    ImGui.set_function("SetNextWindowCollapsed", sol::overload(sol::resolve<void(bool)>(SetNextWindowCollapsed),
                                                               sol::resolve<void(bool, int)>(SetNextWindowCollapsed)));
    ImGui.set_function("SetNextWindowFocus", SetNextWindowFocus);
    ImGui.set_function("SetNextWindowBgAlpha", SetNextWindowBgAlpha);
    ImGui.set_function("SetWindowPos",
                       sol::overload(sol::resolve<void(float, float)>(SetWindowPos),
                                     sol::resolve<void(float, float, int)>(SetWindowPos),
                                     sol::resolve<void(const std::string&, float, float)>(SetWindowPos),
                                     sol::resolve<void(const std::string&, float, float, int)>(SetWindowPos)));
    ImGui.set_function("SetWindowSize",
                       sol::overload(sol::resolve<void(float, float)>(SetWindowSize),
                                     sol::resolve<void(float, float, int)>(SetWindowSize),
                                     sol::resolve<void(const std::string&, float, float)>(SetWindowSize),
                                     sol::resolve<void(const std::string&, float, float, int)>(SetWindowSize)));
    ImGui.set_function(
        "SetWindowCollapsed",
        sol::overload(sol::resolve<void(bool)>(SetWindowCollapsed), sol::resolve<void(bool, int)>(SetWindowCollapsed),
                      sol::resolve<void(const std::string&, bool)>(SetWindowCollapsed),
                      sol::resolve<void(const std::string&, bool, int)>(SetWindowCollapsed)));
    ImGui.set_function("SetWindowFocus", sol::overload(sol::resolve<void()>(SetWindowFocus),
                                                       sol::resolve<void(const std::string&)>(SetWindowFocus)));
    ImGui.set_function("SetWindowFontScale", SetWindowFontScale);
#pragma endregion Window Utilities

#pragma region Content Region
    ImGui.set_function("GetContentRegionMax", GetContentRegionMax);
    ImGui.set_function("GetContentRegionAvail", GetContentRegionAvail);
    ImGui.set_function("GetWindowContentRegionMin", GetWindowContentRegionMin);
    ImGui.set_function("GetWindowContentRegionMax", GetWindowContentRegionMax);
    ImGui.set_function("GetWindowContentRegionWidth", GetWindowContentRegionWidth);
#pragma endregion Content Region

#pragma region Windows Scrolling
    ImGui.set_function("GetScrollX", GetScrollX);
    ImGui.set_function("GetScrollY", GetScrollY);
    ImGui.set_function("GetScrollMaxX", GetScrollMaxX);
    ImGui.set_function("GetScrollMaxY", GetScrollMaxY);
    ImGui.set_function("SetScrollX", SetScrollX);
    ImGui.set_function("SetScrollY", SetScrollY);
    ImGui.set_function("SetScrollHereX",
                       sol::overload(sol::resolve<void()>(SetScrollHereX), sol::resolve<void(float)>(SetScrollHereX)));
    ImGui.set_function("SetScrollHereY",
                       sol::overload(sol::resolve<void()>(SetScrollHereY), sol::resolve<void(float)>(SetScrollHereY)));
    ImGui.set_function("SetScrollFromPosX", sol::overload(sol::resolve<void(float)>(SetScrollFromPosX),
                                                          sol::resolve<void(float, float)>(SetScrollFromPosX)));
    ImGui.set_function("SetScrollFromPosY", sol::overload(sol::resolve<void(float)>(SetScrollFromPosY),
                                                          sol::resolve<void(float, float)>(SetScrollFromPosY)));
#pragma endregion Windows Scrolling

#pragma region Parameters stacks (shared)
    ImGui.set_function("PushFont", PushFont);
    ImGui.set_function("PopFont", PopFont);
#ifdef SOL_IMGUI_USE_COLOR_U32
    ImGui.set_function("PushStyleColor",
                       sol::overload(sol::resolve<void(int, int)>(PushStyleColor),
                                     sol::resolve<void(int, float, float, float, float)>(PushStyleColor)));
#else
    ImGui.set_function("PushStyleColor", PushStyleColor);
#endif
    ImGui.set_function("PopStyleColor",
                       sol::overload(sol::resolve<void()>(PopStyleColor), sol::resolve<void(int)>(PopStyleColor)));
    ImGui.set_function("GetStyleColorVec4", GetStyleColorVec4);
    ImGui.set_function("GetFont", GetFont);
    ImGui.set_function("GetFontSize", GetFontSize);
    ImGui.set_function("GetFontTexUvWhitePixel", GetFontTexUvWhitePixel);
#ifdef SOL_IMGUI_USE_COLOR_U32
    ImGui.set_function("GetColorU32", sol::overload(sol::resolve<int(int, float)>(GetColorU32),
                                                    sol::resolve<int(float, float, float, float)>(GetColorU32),
                                                    sol::resolve<int(int)>(GetColorU32)));
#endif
#pragma endregion Parameters stacks(shared)

#pragma region Parameters stacks (current window)
    ImGui.set_function("PushItemWidth", PushItemWidth);
    ImGui.set_function("PopItemWidth", PopItemWidth);
    ImGui.set_function("SetNextItemWidth", SetNextItemWidth);
    ImGui.set_function("CalcItemWidth", CalcItemWidth);
    ImGui.set_function("PushTextWrapPos", sol::overload(sol::resolve<void()>(PushTextWrapPos),
                                                        sol::resolve<void(float)>(PushTextWrapPos)));
    ImGui.set_function("PopTextWrapPos", PopTextWrapPos);
    ImGui.set_function("PushAllowKeyboardFocus", PushAllowKeyboardFocus);
    ImGui.set_function("PopAllowKeyboardFocus", PopAllowKeyboardFocus);
    ImGui.set_function("PushButtonRepeat", PushButtonRepeat);
    ImGui.set_function("PopButtonRepeat", PopButtonRepeat);
#pragma endregion Parameters stacks(current window)

#pragma region Cursor / Layout
    ImGui.set_function("Separator", Separator);
    ImGui.set_function("SameLine", sol::overload(sol::resolve<void()>(SameLine), sol::resolve<void(float)>(SameLine)));
    ImGui.set_function("NewLine", NewLine);
    ImGui.set_function("Spacing", Spacing);
    ImGui.set_function("Dummy", Dummy);
    ImGui.set_function("Indent", sol::overload(sol::resolve<void()>(Indent), sol::resolve<void(float)>(Indent)));
    ImGui.set_function("Unindent", sol::overload(sol::resolve<void()>(Unindent), sol::resolve<void(float)>(Unindent)));
    ImGui.set_function("BeginGroup", BeginGroup);
    ImGui.set_function("EndGroup", EndGroup);
    ImGui.set_function("GetCursorPos", GetCursorPos);
    ImGui.set_function("GetCursorPosX", GetCursorPosX);
    ImGui.set_function("GetCursorPosY", GetCursorPosY);
    ImGui.set_function("SetCursorPos", SetCursorPos);
    ImGui.set_function("SetCursorPosX", SetCursorPosX);
    ImGui.set_function("SetCursorPosY", SetCursorPosY);
    ImGui.set_function("GetCursorStartPos", GetCursorStartPos);
    ImGui.set_function("GetCursorScreenPos", GetCursorScreenPos);
    ImGui.set_function("SetCursorScreenPos", SetCursorScreenPos);
    ImGui.set_function("AlignTextToFramePadding", AlignTextToFramePadding);
    ImGui.set_function("GetTextLineHeight", GetTextLineHeight);
    ImGui.set_function("GetTextLineHeightWithSpacing", GetTextLineHeightWithSpacing);
    ImGui.set_function("GetFrameHeight", GetFrameHeight);
    ImGui.set_function("GetFrameHeightWithSpacing", GetFrameHeightWithSpacing);
#pragma endregion Cursor / Layout

#pragma region ID stack / scopes
    ImGui.set_function("PushID", sol::overload(sol::resolve<void(const std::string&)>(PushID),
                                               sol::resolve<void(const std::string&, const std::string&)>(PushID),
                                               sol::resolve<void(int)>(PushID)));
    ImGui.set_function("PopID", PopID);
    ImGui.set_function("GetID", sol::overload(sol::resolve<int(const std::string&)>(GetID),
                                              sol::resolve<int(const std::string&, const std::string&)>(GetID)));
#pragma endregion ID stack / scopes

#pragma region Widgets: Text
    ImGui.set_function("TextUnformatted",
                       sol::overload(sol::resolve<void(const std::string&)>(TextUnformatted),
                                     sol::resolve<void(const std::string&, const std::string&)>(TextUnformatted)));
    ImGui.set_function("Text", Text);
    ImGui.set_function("TextColored", TextColored);
    ImGui.set_function("TextDisabled", TextDisabled);
    ImGui.set_function("TextWrapped", TextWrapped);
    ImGui.set_function("LabelText", LabelText);
    ImGui.set_function("BulletText", BulletText);
#pragma endregion Widgets : Text

#pragma region Widgets: Main
    ImGui.set_function("Button", sol::overload(sol::resolve<bool(const std::string&)>(Button),
                                               sol::resolve<bool(const std::string&, float, float)>(Button)));
    ImGui.set_function("SmallButton", SmallButton);
    ImGui.set_function("InvisibleButton", InvisibleButton);
    ImGui.set_function("ArrowButton", ArrowButton);
    ImGui.set_function("Checkbox", Checkbox);
    ImGui.set_function("RadioButton",
                       sol::overload(sol::resolve<bool(const std::string&, bool)>(RadioButton),
                                     sol::resolve<std::tuple<int, bool>(const std::string&, int, int)>(RadioButton)));
    ImGui.set_function(
        "ProgressBar",
        sol::overload(sol::resolve<void(float)>(ProgressBar), sol::resolve<void(float, float, float)>(ProgressBar),
                      sol::resolve<void(float, float, float, const std::string&)>(ProgressBar)));
    ImGui.set_function("Bullet", Bullet);
#pragma endregion Widgets : Main

#pragma region Widgets: Combo Box
    ImGui.set_function("BeginCombo",
                       sol::overload(sol::resolve<bool(const std::string&, const std::string&)>(BeginCombo),
                                     sol::resolve<bool(const std::string&, const std::string&, int)>(BeginCombo)));
    ImGui.set_function("EndCombo", EndCombo);
    ImGui.set_function(
        "Combo",
        sol::overload(sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int)>(Combo),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int, int)>(Combo),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, const std::string&)>(Combo),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, const std::string&, int)>(Combo)));
#pragma endregion Widgets : Combo Box

#pragma region Widgets: Drags
    ImGui.set_function(
        "DragFloat",
        sol::overload(
            sol::resolve<std::tuple<float, bool>(const std::string&, float)>(DragFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(DragFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(DragFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float)>(DragFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, const std::string&)>(
                DragFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, const std::string&,
                                                 float)>(DragFloat)));
    ImGui.set_function(
        "DragFloat2",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                DragFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float)>(DragFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float)>(DragFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float, float)>(DragFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, float, const std::string&)>(DragFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, float, const std::string&, float)>(DragFloat2)));
    ImGui.set_function(
        "DragFloat3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                DragFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float)>(DragFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float)>(DragFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float, float)>(DragFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, float, const std::string&)>(DragFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, float, const std::string&, float)>(DragFloat3)));
    ImGui.set_function(
        "DragFloat4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                DragFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float)>(DragFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float)>(DragFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float, float)>(DragFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, float, const std::string&)>(DragFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, float, const std::string&, float)>(DragFloat4)));
    ImGui.set_function(
        "DragInt",
        sol::overload(sol::resolve<std::tuple<int, bool>(const std::string&, int)>(DragInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, float)>(DragInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int)>(DragInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int, int)>(DragInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int, int, const std::string&)>(
                          DragInt)));
    ImGui.set_function(
        "DragInt2",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(
                DragInt2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float)>(DragInt2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float, int)>(DragInt2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float, int, int)>(DragInt2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                const std::string&, const sol::table&, float, int, int, const std::string&)>(DragInt2)));
    ImGui.set_function(
        "DragInt3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(
                DragInt3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float)>(DragInt3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float, int)>(DragInt3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float, int, int)>(DragInt3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                const std::string&, const sol::table&, float, int, int, const std::string&)>(DragInt3)));
    ImGui.set_function(
        "DragInt4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(
                DragInt4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float)>(DragInt4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float, int)>(DragInt4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             float, int, int)>(DragInt4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                const std::string&, const sol::table&, float, int, int, const std::string&)>(DragInt4)));
#pragma endregion Widgets : Drags

#pragma region Widgets: Sliders
    ImGui.set_function(
        "SliderFloat",
        sol::overload(
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(SliderFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&)>(
                SliderFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&, float)>(
                SliderFloat)));
    ImGui.set_function(
        "SliderFloat2",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float)>(SliderFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, const std::string&)>(SliderFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, const std::string&, float)>(SliderFloat2)));
    ImGui.set_function(
        "SliderFloat3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float)>(SliderFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, const std::string&)>(SliderFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, const std::string&, float)>(SliderFloat3)));
    ImGui.set_function(
        "SliderFloat4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               float, float)>(SliderFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, const std::string&)>(SliderFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(
                const std::string&, const sol::table&, float, float, const std::string&, float)>(SliderFloat4)));
    ImGui.set_function(
        "SliderAngle",
        sol::overload(
            sol::resolve<std::tuple<float, bool>(const std::string&, float)>(SliderAngle),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(SliderAngle),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(SliderAngle),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&)>(
                SliderAngle)));
    ImGui.set_function(
        "SliderInt",
        sol::overload(
            sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int)>(SliderInt),
            sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int, const std::string&)>(SliderInt)));
    ImGui.set_function(
        "SliderInt2",
        sol::overload(sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                          const std::string&, const sol::table&, int, int)>(SliderInt2),
                      sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                          const std::string&, const sol::table&, int, int, const std::string&)>(SliderInt2)));
    ImGui.set_function(
        "SliderInt3",
        sol::overload(sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                          const std::string&, const sol::table&, int, int)>(SliderInt3),
                      sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                          const std::string&, const sol::table&, int, int, const std::string&)>(SliderInt3)));
    ImGui.set_function(
        "SliderInt4",
        sol::overload(sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                          const std::string&, const sol::table&, int, int)>(SliderInt4),
                      sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(
                          const std::string&, const sol::table&, int, int, const std::string&)>(SliderInt4)));
    ImGui.set_function(
        "VSliderFloat",
        sol::overload(
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, float)>(VSliderFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, float,
                                                 const std::string&)>(VSliderFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, float,
                                                 const std::string&, float)>(VSliderFloat)));
    ImGui.set_function(
        "VSliderInt",
        sol::overload(
            sol::resolve<std::tuple<int, bool>(const std::string&, float, float, int, int, int)>(VSliderInt),
            sol::resolve<std::tuple<int, bool>(const std::string&, float, float, int, int, int, const std::string&)>(
                VSliderInt)));
#pragma endregion Widgets : Sliders

#pragma region Widgets: Inputs using Keyboard
    ImGui.set_function(
        "InputText",
        sol::overload(
            sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int)>(InputText),
            sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int, int)>(
                InputText)));
    ImGui.set_function(
        "InputTextMultiline",
        sol::overload(
            sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int)>(
                InputTextMultiline),
            sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int, float, float)>(
                InputTextMultiline),
            sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int, float, float,
                                                       int)>(InputTextMultiline)));
    ImGui.set_function(
        "InputTextWithHint",
        sol::overload(sol::resolve<std::tuple<std::string, bool>(const std::string&, const std::string&, std::string,
                                                                 unsigned int)>(InputTextWithHint),
                      sol::resolve<std::tuple<std::string, bool>(const std::string&, const std::string&, std::string,
                                                                 unsigned int, int)>(InputTextWithHint)));
    ImGui.set_function(
        "InputFloat",
        sol::overload(
            sol::resolve<std::tuple<float, bool>(const std::string&, float)>(InputFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(InputFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(InputFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&)>(
                InputFloat),
            sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&, int)>(
                InputFloat)));
    ImGui.set_function(
        "InputFloat2",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                InputFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               const std::string&)>(InputFloat2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               const std::string&, int)>(InputFloat2)));
    ImGui.set_function(
        "InputFloat3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                InputFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               const std::string&)>(InputFloat3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               const std::string&, int)>(InputFloat3)));
    ImGui.set_function(
        "InputFloat4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                InputFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               const std::string&)>(InputFloat4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               const std::string&, int)>(InputFloat4)));
    ImGui.set_function(
        "InputInt",
        sol::overload(sol::resolve<std::tuple<int, bool>(const std::string&, int)>(InputInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, int)>(InputInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int)>(InputInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int)>(InputInt),
                      sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int, int)>(InputInt)));
    ImGui.set_function(
        "InputInt2",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(
                InputInt2),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             int)>(InputInt2)));
    ImGui.set_function(
        "InputInt3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(
                InputInt3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             int)>(InputInt3)));
    ImGui.set_function(
        "InputInt4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(
                InputInt4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&,
                                                                             int)>(InputInt4)));
    ImGui.set_function(
        "InputDouble",
        sol::overload(
            sol::resolve<std::tuple<double, bool>(const std::string&, double)>(InputDouble),
            sol::resolve<std::tuple<double, bool>(const std::string&, double, double)>(InputDouble),
            sol::resolve<std::tuple<double, bool>(const std::string&, double, double, double)>(InputDouble),
            sol::resolve<std::tuple<double, bool>(const std::string&, double, double, double, const std::string&)>(
                InputDouble),
            sol::resolve<std::tuple<double, bool>(const std::string&, double, double, double, const std::string&, int)>(
                InputDouble)));
#pragma endregion Widgets : Inputs using Keyboard

#pragma region Widgets: Color Editor / Picker
    ImGui.set_function(
        "ColorEdit3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                ColorEdit3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               int)>(ColorEdit3)));
    ImGui.set_function(
        "ColorEdit4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                ColorEdit4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               int)>(ColorEdit4)));
    ImGui.set_function(
        "ColorPicker3",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                ColorPicker3),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               int)>(ColorPicker3)));
    ImGui.set_function(
        "ColorPicker4",
        sol::overload(
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(
                ColorPicker4),
            sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&,
                                                                               int)>(ColorPicker4)));
#pragma endregion Widgets : Color Editor / Picker

#pragma region Widgets: Trees
    ImGui.set_function("TreeNode", sol::overload(sol::resolve<bool(const std::string&)>(TreeNode),
                                                 sol::resolve<bool(const std::string&, const std::string&)>(TreeNode)));
    ImGui.set_function("TreeNodeEx",
                       sol::overload(sol::resolve<bool(const std::string&)>(TreeNodeEx),
                                     sol::resolve<bool(const std::string&, int)>(TreeNodeEx),
                                     sol::resolve<bool(const std::string&, int, const std::string&)>(TreeNodeEx)));
    ImGui.set_function("TreePush", TreePush);
    ImGui.set_function("GetTreeNodeToLabelSpacing", GetTreeNodeToLabelSpacing);
    ImGui.set_function(
        "CollapsingHeader",
        sol::overload(sol::resolve<bool(const std::string&)>(CollapsingHeader),
                      sol::resolve<bool(const std::string&, int)>(CollapsingHeader),
                      sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(CollapsingHeader),
                      sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int)>(CollapsingHeader)));
    ImGui.set_function("SetNextItemOpen", sol::overload(sol::resolve<void(bool)>(SetNextItemOpen),
                                                        sol::resolve<void(bool, int)>(SetNextItemOpen)));
#pragma endregion Widgets : Trees

#pragma region Widgets: Selectables
    ImGui.set_function("Selectable",
                       sol::overload(sol::resolve<bool(const std::string&)>(Selectable),
                                     sol::resolve<bool(const std::string&, bool)>(Selectable),
                                     sol::resolve<bool(const std::string&, bool, int)>(Selectable),
                                     sol::resolve<bool(const std::string&, bool, int, float, float)>(Selectable)));
#pragma endregion Widgets : Selectables

#pragma region Widgets: List Boxes
    ImGui.set_function(
        "ListBox",
        sol::overload(
            sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int)>(ListBox),
            sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int, int)>(ListBox)));
    ImGui.set_function("ListBoxHeader",
                       sol::overload(sol::resolve<bool(const std::string&, float, float)>(ListBoxHeader),
                                     sol::resolve<bool(const std::string&, int)>(ListBoxHeader),
                                     sol::resolve<bool(const std::string&, int, int)>(ListBoxHeader)));
    ImGui.set_function("ListBoxFooter", ListBoxFooter);
#pragma endregion Widgets : List Boxes

#pragma region Widgets: Value() Helpers
    ImGui.set_function("Value",
                       sol::overload(sol::resolve<void(const std::string&, bool)>(Value),
                                     sol::resolve<void(const std::string&, int)>(Value),
                                     sol::resolve<void(const std::string&, unsigned int)>(Value),
                                     sol::resolve<void(const std::string&, float)>(Value),
                                     sol::resolve<void(const std::string&, float, const std::string&)>(Value)));
#pragma endregion Widgets : Value() Helpers

#pragma region Widgets: Menu
    ImGui.set_function("BeginMenuBar", BeginMenuBar);
    ImGui.set_function("EndMenuBar", EndMenuBar);
    ImGui.set_function("BeginMainMenuBar", BeginMainMenuBar);
    ImGui.set_function("EndMainMenuBar", EndMainMenuBar);
    ImGui.set_function("BeginMenu", sol::overload(sol::resolve<bool(const std::string&)>(BeginMenu),
                                                  sol::resolve<bool(const std::string&, bool)>(BeginMenu)));
    ImGui.set_function("EndMenu", EndMenu);
    ImGui.set_function(
        "MenuItem",
        sol::overload(
            sol::resolve<bool(const std::string&)>(MenuItem),
            sol::resolve<bool(const std::string&, const std::string&)>(MenuItem),
            sol::resolve<std::tuple<bool, bool>(const std::string&, const std::string&, bool)>(MenuItem),
            sol::resolve<std::tuple<bool, bool>(const std::string&, const std::string&, bool, bool)>(MenuItem)));
#pragma endregion Widgets : Menu

#pragma region Tooltips
    ImGui.set_function("BeginTooltip", BeginTooltip);
    ImGui.set_function("EndTooltip", EndTooltip);
    ImGui.set_function("SetTooltip", SetTooltip);
#pragma endregion Tooltips

#pragma region Popups, Modals
    ImGui.set_function("BeginPopup", sol::overload(sol::resolve<bool(const std::string&)>(BeginPopup),
                                                   sol::resolve<bool(const std::string&, int)>(BeginPopup)));
    ImGui.set_function("BeginPopupModal",
                       sol::overload(sol::resolve<bool(const std::string&)>(BeginPopupModal),
                                     sol::resolve<bool(const std::string&, bool)>(BeginPopupModal),
                                     sol::resolve<bool(const std::string&, bool, int)>(BeginPopupModal)));
    ImGui.set_function("EndPopup", EndPopup);
    ImGui.set_function("OpenPopup", sol::overload(sol::resolve<void(const std::string&)>(OpenPopup),
                                                  sol::resolve<void(const std::string&, int)>(OpenPopup)));
    ImGui.set_function("OpenPopupContextItem",
                       sol::overload(sol::resolve<void()>(OpenPopupOnItemClick),
                                     sol::resolve<void(const std::string&)>(OpenPopupContextItem),
                                     sol::resolve<bool(const std::string&, int)>(OpenPopupContextItem)));
    ImGui.set_function("CloseCurrentPopup", CloseCurrentPopup);
    ImGui.set_function("BeginPopupContextItem",
                       sol::overload(sol::resolve<bool()>(BeginPopupContextItem),
                                     sol::resolve<bool(const std::string&)>(BeginPopupContextItem),
                                     sol::resolve<bool(const std::string&, int)>(BeginPopupContextItem)));
    ImGui.set_function("BeginPopupContextWindow",
                       sol::overload(sol::resolve<bool()>(BeginPopupContextWindow),
                                     sol::resolve<bool(const std::string&)>(BeginPopupContextWindow),
                                     sol::resolve<bool(const std::string&, int)>(BeginPopupContextWindow)));
    ImGui.set_function("BeginPopupContextVoid",
                       sol::overload(sol::resolve<bool()>(BeginPopupContextVoid),
                                     sol::resolve<bool(const std::string&)>(BeginPopupContextVoid),
                                     sol::resolve<bool(const std::string&, int)>(BeginPopupContextVoid)));
    ImGui.set_function("IsPopupOpen", sol::overload(sol::resolve<bool(const std::string&)>(IsPopupOpen),
                                                    sol::resolve<bool(const std::string&, int)>(IsPopupOpen)));
#pragma endregion Popups, Modals

#pragma region Columns
    ImGui.set_function("Columns", sol::overload(sol::resolve<void()>(Columns), sol::resolve<void(int)>(Columns),
                                                sol::resolve<void(int, const std::string&)>(Columns),
                                                sol::resolve<void(int, const std::string&, bool)>(Columns)));
    ImGui.set_function("NextColumn", NextColumn);
    ImGui.set_function("GetColumnIndex", GetColumnIndex);
    ImGui.set_function("GetColumnWidth",
                       sol::overload(sol::resolve<float()>(GetColumnWidth), sol::resolve<float(int)>(GetColumnWidth)));
    ImGui.set_function("SetColumnWidth", SetColumnWidth);
    ImGui.set_function("GetColumnOffset", sol::overload(sol::resolve<float()>(GetColumnOffset),
                                                        sol::resolve<float(int)>(GetColumnOffset)));
    ImGui.set_function("SetColumnOffset", SetColumnOffset);
    ImGui.set_function("GetColumnsCount", GetColumnsCount);
#pragma endregion Columns

#pragma region Tab Bars, Tabs
    ImGui.set_function("BeginTabBar", sol::overload(sol::resolve<bool(const std::string&)>(BeginTabBar),
                                                    sol::resolve<bool(const std::string&, int)>(BeginTabBar)));
    ImGui.set_function("EndTabBar", EndTabBar);
    ImGui.set_function(
        "BeginTabItem",
        sol::overload(sol::resolve<bool(const std::string&)>(BeginTabItem),
                      sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(BeginTabItem),
                      sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int)>(BeginTabItem)));
    ImGui.set_function("EndTabItem", EndTabItem);
    ImGui.set_function("SetTabItemClosed", SetTabItemClosed);
#pragma endregion Tab Bars, Tabs

#pragma region Docking
#ifdef IMGUI_HAS_DOCK
    ImGui.set_function("DockSpace", sol::overload(sol::resolve<void(unsigned int)>(DockSpace),
                                                  sol::resolve<void(unsigned int, float, float)>(DockSpace),
                                                  sol::resolve<void(unsigned int, float, float, int)>(DockSpace)));
    ImGui.set_function("SetNextWindowDockID",
                       sol::overload(sol::resolve<void(unsigned int)>(SetNextWindowDockID),
                                     sol::resolve<void(unsigned int, int)>(SetNextWindowDockID)));
    ImGui.set_function("GetWindowDockID", GetWindowDockID);
    ImGui.set_function("IsWindowDocked", IsWindowDocked);
#endif  // IMGUI_HAS_DOCK
#pragma endregion Docking

#pragma region Logging / Capture
    ImGui.set_function("LogToTTY", sol::overload(sol::resolve<void()>(LogToTTY), sol::resolve<void(int)>(LogToTTY)));
    ImGui.set_function("LogToFile", sol::overload(sol::resolve<void(int)>(LogToFile),
                                                  sol::resolve<void(int, const std::string&)>(LogToFile)));
    ImGui.set_function("LogToClipboard",
                       sol::overload(sol::resolve<void()>(LogToClipboard), sol::resolve<void(int)>(LogToClipboard)));
    ImGui.set_function("LogFinish", LogFinish);
    ImGui.set_function("LogButtons", LogButtons);
    ImGui.set_function("LogText", LogText);
#pragma endregion Logging / Capture

#pragma region Clipping
    ImGui.set_function("PushClipRect", PushClipRect);
    ImGui.set_function("PopClipRect", PopClipRect);
#pragma endregion Clipping

#pragma region Focus, Activation
    ImGui.set_function("SetItemDefaultFocus", SetItemDefaultFocus);
    ImGui.set_function("SetKeyboardFocusHere", sol::overload(sol::resolve<void()>(SetKeyboardFocusHere),
                                                             sol::resolve<void(int)>(SetKeyboardFocusHere)));
#pragma endregion Focus, Activation

#pragma region Item/Widgets Utilities
    ImGui.set_function("IsItemHovered",
                       sol::overload(sol::resolve<bool()>(IsItemHovered), sol::resolve<bool(int)>(IsItemHovered)));
    ImGui.set_function("IsItemActive", IsItemActive);
    ImGui.set_function("IsItemFocused", IsItemFocused);
    ImGui.set_function("IsItemClicked",
                       sol::overload(sol::resolve<bool()>(IsItemClicked), sol::resolve<bool(int)>(IsItemClicked)));
    ImGui.set_function("IsItemVisible", IsItemVisible);
    ImGui.set_function("IsItemEdited", IsItemEdited);
    ImGui.set_function("IsItemActivated", IsItemActivated);
    ImGui.set_function("IsItemDeactivated", IsItemDeactivated);
    ImGui.set_function("IsItemDeactivatedAfterEdit", IsItemDeactivatedAfterEdit);
    ImGui.set_function("IsItemToggledOpen", IsItemToggledOpen);
    ImGui.set_function("IsAnyItemHovered", IsAnyItemHovered);
    ImGui.set_function("IsAnyItemActive", IsAnyItemActive);
    ImGui.set_function("IsAnyItemFocused", IsAnyItemFocused);
    ImGui.set_function("GetItemRectMin", GetItemRectMin);
    ImGui.set_function("GetItemRectMax", GetItemRectMax);
    ImGui.set_function("GetItemRectSize", GetItemRectSize);
    ImGui.set_function("SetItemAllowOverlap", SetItemAllowOverlap);
#pragma endregion Item / Widgets Utilities

#pragma region Miscellaneous Utilities
    ImGui.set_function("IsRectVisible", sol::overload(sol::resolve<bool(float, float)>(IsRectVisible),
                                                      sol::resolve<bool(float, float, float, float)>(IsRectVisible)));
    ImGui.set_function("GetTime", GetTime);
    ImGui.set_function("GetFrameCount", GetFrameCount);
    ImGui.set_function("GetStyleColorName", GetStyleColorName);
    ImGui.set_function("BeginChildFrame",
                       sol::overload(sol::resolve<bool(unsigned int, float, float)>(BeginChildFrame),
                                     sol::resolve<bool(unsigned int, float, float, int)>(BeginChildFrame)));
    ImGui.set_function("EndChildFrame", EndChildFrame);
#pragma endregion Miscellaneous Utilities

#pragma region Text Utilities
    ImGui.set_function(
        "CalcTextSize",
        sol::overload(
            sol::resolve<std::tuple<float, float>(const std::string&)>(CalcTextSize),
            sol::resolve<std::tuple<float, float>(const std::string&, const std::string&)>(CalcTextSize),
            sol::resolve<std::tuple<float, float>(const std::string&, const std::string&, bool)>(CalcTextSize),
            sol::resolve<std::tuple<float, float>(const std::string&, const std::string&, bool, float)>(CalcTextSize)));
#pragma endregion Text Utilities

#pragma region Color Utilities
#ifdef SOL_IMGUI_USE_COLOR_U32
    ImGui.set_function("ColorConvertU32ToFloat4", ColorConvertU32ToFloat4);
    ImGui.set_function("ColorConvertFloat4ToU32", ColorConvertFloat4ToU32);
#endif
    ImGui.set_function("ColorConvertRGBtoHSV", ColorConvertRGBtoHSV);
    ImGui.set_function("ColorConvertHSVtoRGB", ColorConvertHSVtoRGB);
#pragma endregion Color Utilities

#pragma region Inputs Utilities: Keyboard
    ImGui.set_function("GetKeyIndex", GetKeyIndex);
    ImGui.set_function("IsKeyDown", IsKeyDown);
    ImGui.set_function("IsKeyPressed", sol::overload(sol::resolve<bool(int)>(IsKeyPressed),
                                                     sol::resolve<bool(int, bool)>(IsKeyPressed)));
    ImGui.set_function("IsKeyReleased", IsKeyReleased);
    ImGui.set_function("CaptureKeyboardFromApp", sol::overload(sol::resolve<void()>(CaptureKeyboardFromApp),
                                                               sol::resolve<void(bool)>(CaptureKeyboardFromApp)));
#pragma endregion Inputs Utilities : Keyboard

#pragma region Inputs Utilities: Mouse
    ImGui.set_function("IsMouseDown", IsMouseDown);
    ImGui.set_function("IsMouseClicked", sol::overload(sol::resolve<bool(int)>(IsMouseClicked),
                                                       sol::resolve<bool(int, bool)>(IsMouseClicked)));
    ImGui.set_function("IsMouseReleased", IsMouseReleased);
    ImGui.set_function("IsMouseDoubleClicked", IsMouseDoubleClicked);
    ImGui.set_function("IsMouseHoveringRect",
                       sol::overload(sol::resolve<bool(float, float, float, float)>(IsMouseHoveringRect),
                                     sol::resolve<bool(float, float, float, float, bool)>(IsMouseHoveringRect)));
    ImGui.set_function("IsAnyMouseDown", IsAnyMouseDown);
    ImGui.set_function("GetMousePos", GetMousePos);
    ImGui.set_function("GetMousePosOnOpeningCurrentPopup", GetMousePosOnOpeningCurrentPopup);
    ImGui.set_function("IsMouseDragging", sol::overload(sol::resolve<bool(int)>(IsMouseDragging),
                                                        sol::resolve<bool(int, float)>(IsMouseDragging)));
    ImGui.set_function("GetMouseDragDelta",
                       sol::overload(sol::resolve<std::tuple<float, float>()>(GetMouseDragDelta),
                                     sol::resolve<std::tuple<float, float>(int)>(GetMouseDragDelta),
                                     sol::resolve<std::tuple<float, float>(int, float)>(GetMouseDragDelta)));
    ImGui.set_function("ResetMouseDragDelta", sol::overload(sol::resolve<void()>(ResetMouseDragDelta),
                                                            sol::resolve<void(int)>(ResetMouseDragDelta)));
    ImGui.set_function("GetMouseCursor", GetMouseCursor);
    ImGui.set_function("SetMouseCursor", SetMouseCursor);
    ImGui.set_function("CaptureMouseFromApp", sol::overload(sol::resolve<void()>(CaptureMouseFromApp),
                                                            sol::resolve<void(bool)>(CaptureMouseFromApp)));
#pragma endregion Inputs Utilities : Mouse

#pragma region Clipboard Utilities
    ImGui.set_function("GetClipboardText", GetClipboardText);
    ImGui.set_function("SetClipboardText", SetClipboardText);
#pragma endregion Clipboard Utilities
}
}  // namespace cqsp::client::scripting
