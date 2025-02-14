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

#include <imgui.h>

#include <string>
#include <utility>

#include <sol/sol.hpp>

namespace sol_ImGui {
// Windows
bool Begin(const std::string& name);
std::tuple<bool, bool> Begin(const std::string& name, bool open);
std::tuple<bool, bool> Begin(const std::string& name, bool open, int flags);
void End();

// Child Windows
bool BeginChild(const std::string& name);
bool BeginChild(const std::string& name, float sizeX);
bool BeginChild(const std::string& name, float sizeX, float sizeY);
bool BeginChild(const std::string& name, float sizeX, float sizeY, bool border);
bool BeginChild(const std::string& name, float sizeX, float sizeY, bool border, int flags);
void EndChild();

// Windows Utilities
bool IsWindowAppearing();
bool IsWindowCollapsed();
bool IsWindowFocused();
bool IsWindowFocused(int flags);
bool IsWindowHovered();
bool IsWindowHovered(int flags);
ImDrawList* GetWindowDrawList();
float GetWindowDpiScale();  // TODO: Unsupported for now
ImGuiViewport* GetWindowViewport();
std::tuple<float, float> GetWindowPos();
std::tuple<float, float> GetWindowSize();
float GetWindowWidth();
float GetWindowHeight();

// Prefer using SetNext...
void SetNextWindowPos(float posX, float posY);
void SetNextWindowPos(float posX, float posY, int cond);
void SetNextWindowPos(float posX, float posY, int cond, float pivotX, float pivotY);
void SetNextWindowSize(float sizeX, float sizeY);
void SetNextWindowSize(float sizeX, float sizeY, int cond);
void SetNextWindowSizeConstraints(float minX, float minY, float maxX, float maxY);
void SetNextWindowContentSize(float sizeX, float sizeY);
void SetNextWindowCollapsed(bool collapsed);
void SetNextWindowCollapsed(bool collapsed, int cond);
void SetNextWindowFocus();
void SetNextWindowBgAlpha(float alpha);
void SetWindowPos(float posX, float posY);
void SetWindowPos(float posX, float posY, int cond);
void SetWindowSize(float sizeX, float sizeY);
void SetWindowSize(float sizeX, float sizeY, int cond);
void SetWindowCollapsed(bool collapsed);
void SetWindowCollapsed(bool collapsed, int cond);
void SetWindowFocus();
void SetWindowFontScale(float scale);
void SetWindowPos(const std::string& name, float posX, float posY);
void SetWindowPos(const std::string& name, float posX, float posY, int cond);
void SetWindowSize(const std::string& name, float sizeX, float sizeY);
void SetWindowSize(const std::string& name, float sizeX, float sizeY, int cond);
void SetWindowCollapsed(const std::string& name, bool collapsed);
void SetWindowCollapsed(const std::string& name, bool collapsed, int cond);
void SetWindowFocus(const std::string& name);

// Content Region
std::tuple<float, float> GetContentRegionMax();
std::tuple<float, float> GetContentRegionAvail();
std::tuple<float, float> GetWindowContentRegionMin();
std::tuple<float, float> GetWindowContentRegionMax();
float GetWindowContentRegionWidth();

std::tuple<float, float> GetDisplaySize();

// Windows Scrolling
float GetScrollX();
float GetScrollY();
float GetScrollMaxX();
float GetScrollMaxY();
void SetScrollX(float scrollX);
void SetScrollY(float scrollY);
void SetScrollHereX();
void SetScrollHereX(float centerXRatio);
void SetScrollHereY();
void SetScrollHereY(float centerYRatio);
void SetScrollFromPosX(float localX);
void SetScrollFromPosX(float localX, float centerXRatio);
void SetScrollFromPosY(float localY);
void SetScrollFromPosY(float localY, float centerYRatio);

// Parameters stacks (shared)
void PushFont(ImFont* pFont);
void PopFont();
#ifdef SOL_IMGUI_USE_COLOR_U32
void PushStyleColor(int idx, int col);
#endif
void PushStyleColor(int idx, float colR, float colG, float colB, float colA);
void PopStyleColor();
void PopStyleColor(int count);
void PushStyleVar(int idx, float val);
void PushStyleVar(int idx, float valX, float valY);
void PopStyleVar();
void PopStyleVar(int count);
std::tuple<float, float, float, float> GetStyleColorVec4(int idx);
ImFont* GetFont();
float GetFontSize();
std::tuple<float, float> GetFontTexUvWhitePixel();
#ifdef SOL_IMGUI_USE_COLOR_U32
int GetColorU32(int idx, float alphaMul);
int GetColorU32(float colR, float colG, float colB, float colA);
int GetColorU32(int col);
#endif

// Parameters stacks (current window)
void PushItemWidth(float itemWidth);
void PopItemWidth();
void SetNextItemWidth(float itemWidth);
float CalcItemWidth();
void PushTextWrapPos();
void PushTextWrapPos(float wrapLocalPosX);
void PopTextWrapPos();
void PushAllowKeyboardFocus(bool allowKeyboardFocus);
void PopAllowKeyboardFocus();
void PushButtonRepeat(bool repeat);
void PopButtonRepeat();

// Cursor / Layout
void Separator();
void SameLine();
void SameLine(float offsetFromStartX);
void SameLine(float offsetFromStartX, float spacing);
void NewLine();
void Spacing();
void Dummy(float sizeX, float sizeY);
void Indent();
void Indent(float indentW);
void Unindent();
void Unindent(float indentW);
void BeginGroup();
void EndGroup();
std::tuple<float, float> GetCursorPos();
float GetCursorPosX();
float GetCursorPosY();
void SetCursorPos(float localX, float localY);
void SetCursorPosX(float localX);
void SetCursorPosY(float localY);
std::tuple<float, float> GetCursorStartPos();
std::tuple<float, float> GetCursorScreenPos();
void SetCursorScreenPos(float posX, float posY);
void AlignTextToFramePadding();
float GetTextLineHeight();
float GetTextLineHeightWithSpacing();
float GetFrameHeight();
float GetFrameHeightWithSpacing();

// ID stack / scopes
void PushID(const std::string& stringID);
void PushID(const std::string& stringIDBegin, const std::string& stringIDEnd);
void PushID(const void*);
void PushID(int intID);
void PopID();
int GetID(const std::string& stringID);
int GetID(const std::string& stringIDBegin, const std::string& stringIDEnd);
int GetID(const void*);

// Widgets: Text
void TextUnformatted(const std::string& text);
void TextUnformatted(const std::string& text, const std::string& textEnd);
void Text(const std::string& text);
void TextColored(float colR, float colG, float colB, float colA, const std::string& text);
void TextDisabled(const std::string& text);
void TextWrapped(const std::string text);
void LabelText(const std::string& label, const std::string& text);
void BulletText(const std::string& text);

// Widgets: Main
bool Button(const std::string& label);
bool Button(const std::string& label, float sizeX, float sizeY);
bool SmallButton(const std::string& label);
bool InvisibleButton(const std::string& stringID, float sizeX, float sizeY);
bool ArrowButton(const std::string& stringID, int dir);
void Image();
void ImageButton();
std::tuple<bool, bool> Checkbox(const std::string& label, bool v);
bool CheckboxFlags();
bool RadioButton(const std::string& label, bool active);
std::tuple<int, bool> RadioButton(const std::string& label, int v, int vButton);
void ProgressBar(float fraction);
void ProgressBar(float fraction, float sizeX, float sizeY);
void ProgressBar(float fraction, float sizeX, float sizeY, const std::string& overlay);
void Bullet();

// Widgets: Combo Box
bool BeginCombo(const std::string& label, const std::string& previewValue);
bool BeginCombo(const std::string& label, const std::string& previewValue, int flags);
void EndCombo();
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const sol::table& items, int itemsCount);
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const sol::table& items, int itemsCount,
                            int popupMaxHeightInItems);
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const std::string& itemsSeparatedByZeros);
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const std::string& itemsSeparatedByZeros,
                            int popupMaxHeightInItems);
// TODO: 3rd Combo from ImGui not Supported

// Widgets: Drags
std::tuple<float, bool> DragFloat(const std::string& label, float v);
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed);
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min);
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max);
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max,
                                  const std::string& format);
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max,
                                  const std::string& format, float power);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format, float power);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format, float power);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format, float power);
void DragFloatRange2();
std::tuple<int, bool> DragInt(const std::string& label, int v);
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed);
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min);
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max);
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max,
                              const std::string& format);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max,
                                                             const std::string& format);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max,
                                                             const std::string& format);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max);
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max,
                                                             const std::string& format);
void DragIntRange2();
void DragScalar();
void DragScalarN();

// Widgets: Sliders
std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max);
std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max,
                                    const std::string& format);
std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max,
                                    const std::string& format, float power);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format,
                                                                   float power);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format,
                                                                   float power);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format,
                                                                   float power);
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad);
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min);
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max);
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max,
                                    const std::string& format);
std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max);
std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max, const std::string& format);
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max);
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max, const std::string& format);
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max);
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max, const std::string& format);
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max);
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max, const std::string& format);
void SliderScalar();
void SliderScalarN();
std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min,
                                     float v_max);
std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min,
                                     float v_max, const std::string& format);
std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min,
                                     float v_max, const std::string& format, float power);
std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max);
std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max,
                                 const std::string& format);
void VSliderScalar();

// Widgets: Input with Keyboard
std::tuple<std::string, bool> InputText(const std::string& label, std::string text, unsigned int buf_size);
std::tuple<std::string, bool> InputText(const std::string& label, std::string text, unsigned int buf_size, int flags);
std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size);
std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size,
                                                 float sizeX, float sizeY);
std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size,
                                                 float sizeX, float sizeY, int flags);
std::tuple<std::string, bool> InputTextWithHint(const std::string& label, const std::string& hint, std::string text,
                                                unsigned int buf_size);
std::tuple<std::string, bool> InputTextWithHint(const std::string& label, const std::string& hint, std::string text,
                                                unsigned int buf_size, int flags);
std::tuple<float, bool> InputFloat(const std::string& label, float v);
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step);
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast);
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast,
                                   const std::string& format);
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast,
                                   const std::string& format, int flags);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v,
                                                                  const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v,
                                                                  const std::string& format, int flags);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v,
                                                                  const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v,
                                                                  const std::string& format, int flags);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v,
                                                                  const std::string& format);
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v,
                                                                  const std::string& format, int flags);
std::tuple<int, bool> InputInt(const std::string& label, int v);
std::tuple<int, bool> InputInt(const std::string& label, int v, int step);
std::tuple<int, bool> InputInt(const std::string& label, int v, int step, int step_fast);
std::tuple<int, bool> InputInt(const std::string& label, int v, int step, int step_fast, int flags);
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt2(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt2(const std::string& label, const sol::table& v, int flags);
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt3(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt3(const std::string& label, const sol::table& v, int flags);
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt4(const std::string& label, const sol::table& v);
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt4(const std::string& label, const sol::table& v, int flags);
std::tuple<double, bool> InputDouble(const std::string& label, double v);
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step);
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast);
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast,
                                     const std::string& format);
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast,
                                     const std::string& format, int flags);
void InputScalar();
void InputScalarN();

// Widgets: Color Editor / Picker
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col,
                                                                 int flags);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col,
                                                                 int flags);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col,
                                                                   int flags);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col);
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col,
                                                                   int flags);
bool ColorButton(const std::string& desc_id, const sol::table& col);
bool ColorButton(const std::string& desc_id, const sol::table& col, int flags);
bool ColorButton(const std::string& desc_id, const sol::table& col, int flags, float sizeX, float sizeY);
void SetColorEditOptions(int flags);

// Widgets: Trees
bool TreeNode(const std::string& label);
bool TreeNode(const std::string& label, const std::string& fmt);
/* TODO: TreeNodeV(...) (2) ==> UNSUPPORTED */
bool TreeNodeEx(const std::string& label);
bool TreeNodeEx(const std::string& label, int flags);
bool TreeNodeEx(const std::string& label, int flags, const std::string& fmt);
/* TODO: TreeNodeExV(...) (2) ==> UNSUPPORTED */
void TreePush(const std::string& str_id);
/* TODO: TreePush(const void*) ==> UNSUPPORTED */
void TreePop();
float GetTreeNodeToLabelSpacing();
bool CollapsingHeader(const std::string& label);
bool CollapsingHeader(const std::string& label, int flags);
std::tuple<bool, bool> CollapsingHeader(const std::string& label, bool open);
std::tuple<bool, bool> CollapsingHeader(const std::string& label, bool open, int flags);
void SetNextItemOpen(bool is_open);
void SetNextItemOpen(bool is_open, int cond);

// Widgets: Selectables
// TODO: Only one of Selectable variations is possible due to same parameters for Lua
bool Selectable(const std::string& label);
bool Selectable(const std::string& label, bool selected);
bool Selectable(const std::string& label, bool selected, int flags);
bool Selectable(const std::string& label, bool selected, int flags, float sizeX, float sizeY);

// Widgets: List Boxes
std::tuple<int, bool> ListBox(const std::string& label, int current_item, const sol::table& items, int items_count);
std::tuple<int, bool> ListBox(const std::string& label, int current_item, const sol::table& items, int items_count,
                              int height_in_items);
bool ListBoxHeader(const std::string& label, float sizeX, float sizeY);
bool ListBoxHeader(const std::string& label, int items_count);
bool ListBoxHeader(const std::string& label, int items_count, int height_in_items);
void ListBoxFooter();

// Widgets: Data Plotting
/* TODO: Widgets Data Plotting ==> UNSUPPORTED (barely used and quite long functions) */

// Widgets: Value() helpers
void Value(const std::string& prefix, bool b);
void Value(const std::string& prefix, int v);
void Value(const std::string& prefix, unsigned int v);
void Value(const std::string& prefix, float v);
void Value(const std::string& prefix, float v, const std::string& float_format);

// Widgets: Menus
bool BeginMenuBar();
void EndMenuBar();
bool BeginMainMenuBar();
void EndMainMenuBar();
bool BeginMenu(const std::string& label);
bool BeginMenu(const std::string& label, bool enabled);
void EndMenu();
bool MenuItem(const std::string& label);
bool MenuItem(const std::string& label, const std::string& shortcut);
std::tuple<bool, bool> MenuItem(const std::string& label, const std::string& shortcut, bool selected);
std::tuple<bool, bool> MenuItem(const std::string& label, const std::string& shortcut, bool selected, bool enabled);

// Tooltips
void BeginTooltip();
void EndTooltip();
void SetTooltip(const std::string& fmt);
void SetTooltipV();

// Popups, Modals
bool BeginPopup(const std::string& str_id);
bool BeginPopup(const std::string& str_id, int flags);
bool BeginPopupModal(const std::string& name);
bool BeginPopupModal(const std::string& name, bool open);
bool BeginPopupModal(const std::string& name, bool open, int flags);
void EndPopup();
void OpenPopup(const std::string& str_id);
void OpenPopup(const std::string& str_id, int popup_flags);
void OpenPopupOnItemClick();  // TODO: Verify
void OpenPopupContextItem(const std::string& str_id);
bool OpenPopupContextItem(const std::string& str_id, int popup_flags);
void CloseCurrentPopup();
bool BeginPopupContextItem();
bool BeginPopupContextItem(const std::string& str_id);
bool BeginPopupContextItem(const std::string& str_id, int popup_flags);
bool BeginPopupContextWindow();
bool BeginPopupContextWindow(const std::string& str_id);
bool BeginPopupContextWindow(const std::string& str_id, int popup_flags);
bool BeginPopupContextVoid();
bool BeginPopupContextVoid(const std::string& str_id);
bool BeginPopupContextVoid(const std::string& str_id, int popup_flags);
bool IsPopupOpen(const std::string& str_id);
bool IsPopupOpen(const std::string& str_id, int popup_flags);

// Columns
void Columns();
void Columns(int count);
void Columns(int count, const std::string& id);
void Columns(int count, const std::string& id, bool border);
void NextColumn();
int GetColumnIndex();
float GetColumnWidth();
float GetColumnWidth(int column_index);
void SetColumnWidth(int column_index, float width);
float GetColumnOffset();
float GetColumnOffset(int column_index);
void SetColumnOffset(int column_index, float offset_x);
int GetColumnsCount();

// Tab Bars, Tabs
bool BeginTabBar(const std::string& str_id);
bool BeginTabBar(const std::string& str_id, int flags);
void EndTabBar();
bool BeginTabItem(const std::string& label);
std::tuple<bool, bool> BeginTabItem(const std::string& label, bool open);
std::tuple<bool, bool> BeginTabItem(const std::string& label, bool open, int flags);
void EndTabItem();
void SetTabItemClosed(const std::string& tab_or_docked_window_label);
#ifdef IMGUI_HAS_DOCK
// Docking
void DockSpace(unsigned int id);
void DockSpace(unsigned int id, float sizeX, float sizeY);
void DockSpace(unsigned int id, float sizeX, float sizeY, int flags);
unsigned int DockSpaceOverViewport();
void SetNextWindowDockID(unsigned int dock_id);
void SetNextWindowDockID(unsigned int dock_id, int cond);
void SetNextWindowClass();
unsigned int GetWindowDockID();
bool IsWindowDocked();
#endif  // IMGUI_HAS_DOC
// Logging
void LogToTTY();
void LogToTTY(int auto_open_depth);
void LogToFile();
void LogToFile(int auto_open_depth);
void LogToFile(int auto_open_depth, const std::string& filename);
void LogToClipboard();
void LogToClipboard(int auto_open_depth);
void LogFinish();
void LogButtons();
void LogText(const std::string& fmt);

// Drag and Drop
// TODO: Drag and Drop ==> UNSUPPORTED

// Clipping
void PushClipRect(float min_x, float min_y, float max_x, float max_y, bool intersect_current);
void PopClipRect();

// Focus, Activation
void SetItemDefaultFocus();
void SetKeyboardFocusHere();
void SetKeyboardFocusHere(int offset);

// Item/Widgets Utilities
bool IsItemHovered();
bool IsItemHovered(int flags);
bool IsItemActive();
bool IsItemFocused();
bool IsItemClicked();
bool IsItemClicked(int mouse_button);
bool IsItemVisible();
bool IsItemEdited();
bool IsItemActivated();
bool IsItemDeactivated();
bool IsItemDeactivatedAfterEdit();
bool IsItemToggledOpen();
bool IsAnyItemHovered();
bool IsAnyItemActive();
bool IsAnyItemFocused();
std::tuple<float, float> GetItemRectMin();
std::tuple<float, float> GetItemRectMax();
std::tuple<float, float> GetItemRectSize();
void SetItemAllowOverlap();

// Miscellaneous Utilities
bool IsRectVisible(float sizeX, float sizeY);
bool IsRectVisible(float minX, float minY, float maxX, float maxY);
double GetTime();
int GetFrameCount();
/* TODO: GetBackgroundDrawList(), GetForeGroundDrawList(), GetDrawListSharedData() ==> UNSUPPORTED */
std::string GetStyleColorName(int idx);
/* TODO: SetStateStorage(), GetStateStorage(), CalcListClipping() ==> UNSUPPORTED */
bool BeginChildFrame(unsigned int id, float sizeX, float sizeY);
bool BeginChildFrame(unsigned int id, float sizeX, float sizeY, int flags);
void EndChildFrame();

// Text Utilities
std::tuple<float, float> CalcTextSize(const std::string& text);
std::tuple<float, float> CalcTextSize(const std::string& text, const std::string& text_end);
std::tuple<float, float> CalcTextSize(const std::string& text, const std::string& text_end,
                                      bool hide_text_after_double_hash);
std::tuple<float, float> CalcTextSize(const std::string& text, const std::string& text_end,
                                      bool hide_text_after_double_hash, float wrap_width);

// Color Utilities
#ifdef SOL_IMGUI_USE_COLOR_U32
sol::as_table_t<std::vector<float>> ColorConvertU32ToFloat4(unsigned int in);
unsigned int ColorConvertFloat4ToU32(const sol::table& rgba);
#endif
std::tuple<float, float, float> ColorConvertRGBtoHSV(float r, float g, float b);
std::tuple<float, float, float> ColorConvertHSVtoRGB(float h, float s, float v);

// Inputs Utilities: Keyboard
int GetKeyIndex(int imgui_key);
bool IsKeyDown(int user_key_index);
bool IsKeyPressed(int user_key_index);
bool IsKeyPressed(int user_key_index, bool repeat);
bool IsKeyReleased(int user_key_index);
int GetKeyPressedAmount(int key_index, float repeat_delay, float rate);
void CaptureKeyboardFromApp();
void CaptureKeyboardFromApp(bool want_capture_keyboard_value);

// Inputs Utilities: Mouse
bool IsMouseDown(int button);
bool IsMouseClicked(int button);
bool IsMouseClicked(int button, bool repeat);
bool IsMouseReleased(int button);
bool IsMouseDoubleClicked(int button);
bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y);
bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y, bool clip);
bool IsMousePosValid();
bool IsAnyMouseDown();
std::tuple<float, float> GetMousePos();
std::tuple<float, float> GetMousePosOnOpeningCurrentPopup();
bool IsMouseDragging(int button);
bool IsMouseDragging(int button, float lock_threshold);
std::tuple<float, float> GetMouseDragDelta();
std::tuple<float, float> GetMouseDragDelta(int button);
std::tuple<float, float> GetMouseDragDelta(int button, float lock_threshold);
void ResetMouseDragDelta();
void ResetMouseDragDelta(int button);
int GetMouseCursor();
void SetMouseCursor(int cursor_type);
void CaptureMouseFromApp();
void CaptureMouseFromApp(bool want_capture_mouse_value);

// Clipboard Utilities
std::string GetClipboardText();
void SetClipboardText(const std::string& text);
}  // namespace sol_ImGui