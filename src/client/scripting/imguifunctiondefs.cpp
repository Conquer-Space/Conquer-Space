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
#include "client/scripting/imguifunctiondefs.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace cqsp::client::scripting {
// Windows
bool Begin(const std::string& name) { return ImGui::Begin(name.c_str()); }
std::tuple<bool, bool> Begin(const std::string& name, bool open) {
    if (!open) return std::make_tuple(false, false);

    bool shouldDraw = ImGui::Begin(name.c_str(), &open);

    if (!open) {
        ImGui::End();
        return std::make_tuple(false, false);
    }

    return std::make_tuple(open, shouldDraw);
}
std::tuple<bool, bool> Begin(const std::string& name, bool open, int flags) {
    if (!open) return std::make_tuple(false, false);
    bool shouldDraw = ImGui::Begin(name.c_str(), &open, static_cast<ImGuiWindowFlags_>(flags));

    if (!open) {
        ImGui::End();
        return std::make_tuple(false, false);
    }

    return std::make_tuple(open, shouldDraw);
}
void End() { ImGui::End(); }

// Child Windows
bool BeginChild(const std::string& name) { return ImGui::BeginChild(name.c_str()); }
bool BeginChild(const std::string& name, float sizeX) { return ImGui::BeginChild(name.c_str(), {sizeX, 0}); }
bool BeginChild(const std::string& name, float sizeX, float sizeY) {
    return ImGui::BeginChild(name.c_str(), {sizeX, sizeY});
}
bool BeginChild(const std::string& name, float sizeX, float sizeY, bool border) {
    return ImGui::BeginChild(name.c_str(), {sizeX, sizeY}, border);
}
bool BeginChild(const std::string& name, float sizeX, float sizeY, bool border, int flags) {
    return ImGui::BeginChild(name.c_str(), {sizeX, sizeY}, border, static_cast<ImGuiWindowFlags>(flags));
}
void EndChild() { ImGui::EndChild(); }

// Windows Utilities
bool IsWindowAppearing() { return ImGui::IsWindowAppearing(); }
bool IsWindowCollapsed() { return ImGui::IsWindowCollapsed(); }
bool IsWindowFocused() { return ImGui::IsWindowFocused(); }
bool IsWindowFocused(int flags) { return ImGui::IsWindowFocused(static_cast<ImGuiFocusedFlags>(flags)); }
bool IsWindowHovered() { return ImGui::IsWindowHovered(); }
bool IsWindowHovered(int flags) { return ImGui::IsWindowHovered(static_cast<ImGuiHoveredFlags>(flags)); }
ImDrawList* GetWindowDrawList() { return nullptr; /* TODO: GetWindowDrawList() ==> UNSUPPORTED */ }
float GetWindowDpiScale() { return 1; }  // TODO(zyunl): Unsupported for now
ImGuiViewport* GetWindowViewport() { return nullptr; /* TODO: GetWindowViewport() ==> UNSUPPORTED */ }
std::tuple<float, float> GetWindowPos() {
    const auto vec2 {ImGui::GetWindowPos()};
    return std::make_tuple(vec2.x, vec2.y);
}

std::tuple<float, float> GetDisplaySize() {
    return std::make_tuple(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
}

std::tuple<float, float> GetWindowSize() {
    const auto vec2 {ImGui::GetWindowSize()};
    return std::make_tuple(vec2.x, vec2.y);
}
float GetWindowWidth() { return ImGui::GetWindowWidth(); }
float GetWindowHeight() { return ImGui::GetWindowHeight(); }

// Prefer using SetNext...
void SetNextWindowPos(float posX, float posY) { ImGui::SetNextWindowPos({posX, posY}); }
void SetNextWindowPos(float posX, float posY, int cond) {
    ImGui::SetNextWindowPos({posX, posY}, static_cast<ImGuiCond>(cond));
}
void SetNextWindowPos(float posX, float posY, int cond, float pivotX, float pivotY) {
    ImGui::SetNextWindowPos({posX, posY}, static_cast<ImGuiCond>(cond), {pivotX, pivotY});
}
void SetNextWindowSize(float sizeX, float sizeY) { ImGui::SetNextWindowSize({sizeX, sizeY}); }
void SetNextWindowSize(float sizeX, float sizeY, int cond) {
    ImGui::SetNextWindowSize({sizeX, sizeY}, static_cast<ImGuiCond>(cond));
}
void SetNextWindowSizeConstraints(float minX, float minY, float maxX, float maxY) {
    ImGui::SetNextWindowSizeConstraints({minX, minY}, {maxX, maxY});
}
void SetNextWindowContentSize(float sizeX, float sizeY) { ImGui::SetNextWindowContentSize({sizeX, sizeY}); }
void SetNextWindowCollapsed(bool collapsed) { ImGui::SetNextWindowCollapsed(collapsed); }
void SetNextWindowCollapsed(bool collapsed, int cond) {
    ImGui::SetNextWindowCollapsed(collapsed, static_cast<ImGuiCond>(cond));
}
void SetNextWindowFocus() { ImGui::SetNextWindowFocus(); }
void SetNextWindowBgAlpha(float alpha) { ImGui::SetNextWindowBgAlpha(alpha); }
void SetWindowPos(float posX, float posY) { ImGui::SetWindowPos({posX, posY}); }
void SetWindowPos(float posX, float posY, int cond) { ImGui::SetWindowPos({posX, posY}, static_cast<ImGuiCond>(cond)); }
void SetWindowSize(float sizeX, float sizeY) { ImGui::SetWindowSize({sizeX, sizeY}); }
void SetWindowSize(float sizeX, float sizeY, int cond) {
    ImGui::SetWindowSize({sizeX, sizeY}, static_cast<ImGuiCond>(cond));
}
void SetWindowCollapsed(bool collapsed) { ImGui::SetWindowCollapsed(collapsed); }
void SetWindowCollapsed(bool collapsed, int cond) {
    ImGui::SetWindowCollapsed(collapsed, static_cast<ImGuiCond>(cond));
}
void SetWindowFocus() { ImGui::SetWindowFocus(); }
void SetWindowFontScale(float scale) { ImGui::SetWindowFontScale(scale); }
void SetWindowPos(const std::string& name, float posX, float posY) { ImGui::SetWindowPos(name.c_str(), {posX, posY}); }
void SetWindowPos(const std::string& name, float posX, float posY, int cond) {
    ImGui::SetWindowPos(name.c_str(), {posX, posY}, static_cast<ImGuiCond>(cond));
}
void SetWindowSize(const std::string& name, float sizeX, float sizeY) {
    ImGui::SetWindowSize(name.c_str(), {sizeX, sizeY});
}
void SetWindowSize(const std::string& name, float sizeX, float sizeY, int cond) {
    ImGui::SetWindowSize(name.c_str(), {sizeX, sizeY}, static_cast<ImGuiCond>(cond));
}
void SetWindowCollapsed(const std::string& name, bool collapsed) { ImGui::SetWindowCollapsed(name.c_str(), collapsed); }
void SetWindowCollapsed(const std::string& name, bool collapsed, int cond) {
    ImGui::SetWindowCollapsed(name.c_str(), collapsed, static_cast<ImGuiCond>(cond));
}
void SetWindowFocus(const std::string& name) { ImGui::SetWindowFocus(name.c_str()); }

// Content Region
std::tuple<float, float> GetContentRegionMax() {
    const auto vec2 {ImGui::GetContentRegionMax()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetContentRegionAvail() {
    const auto vec2 {ImGui::GetContentRegionAvail()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetWindowContentRegionMin() {
    const auto vec2 {ImGui::GetWindowContentRegionMin()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetWindowContentRegionMax() {
    const auto vec2 {ImGui::GetWindowContentRegionMax()};
    return std::make_tuple(vec2.x, vec2.y);
}
float GetWindowContentRegionWidth() { return ImGui::GetWindowContentRegionWidth(); }

// Windows Scrolling
float GetScrollX() { return ImGui::GetScrollX(); }
float GetScrollY() { return ImGui::GetScrollY(); }
float GetScrollMaxX() { return ImGui::GetScrollMaxX(); }
float GetScrollMaxY() { return ImGui::GetScrollMaxY(); }
void SetScrollX(float scrollX) { ImGui::SetScrollX(scrollX); }
void SetScrollY(float scrollY) { ImGui::SetScrollY(scrollY); }
void SetScrollHereX() { ImGui::SetScrollHereX(); }
void SetScrollHereX(float centerXRatio) { ImGui::SetScrollHereX(centerXRatio); }
void SetScrollHereY() { ImGui::SetScrollHereY(); }
void SetScrollHereY(float centerYRatio) { ImGui::SetScrollHereY(centerYRatio); }
void SetScrollFromPosX(float localX) { ImGui::SetScrollFromPosX(localX); }
void SetScrollFromPosX(float localX, float centerXRatio) { ImGui::SetScrollFromPosX(localX, centerXRatio); }
void SetScrollFromPosY(float localY) { ImGui::SetScrollFromPosY(localY); }
void SetScrollFromPosY(float localY, float centerYRatio) { ImGui::SetScrollFromPosY(localY, centerYRatio); }

// Parameters stacks (shared)
void PushFont(ImFont* pFont) { ImGui::PushFont(pFont); }
void PopFont() { ImGui::PopFont(); }
#ifdef SOL_IMGUI_USE_COLOR_U32
void PushStyleColor(int idx, int col) { ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), ImU32(col)); }
#endif
void PushStyleColor(int idx, float colR, float colG, float colB, float colA) {
    ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), {colR, colG, colB, colA});
}
void PopStyleColor() { ImGui::PopStyleColor(); }
void PopStyleColor(int count) { ImGui::PopStyleColor(count); }
void PushStyleVar(int idx, float val) { ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(idx), val); }
void PushStyleVar(int idx, float valX, float valY) {
    ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(idx), {valX, valY});
}
void PopStyleVar() { ImGui::PopStyleVar(); }
void PopStyleVar(int count) { ImGui::PopStyleVar(count); }
std::tuple<float, float, float, float> GetStyleColorVec4(int idx) {
    const auto col {ImGui::GetStyleColorVec4(static_cast<ImGuiCol>(idx))};
    return std::make_tuple(col.x, col.y, col.z, col.w);
}
ImFont* GetFont() { return ImGui::GetFont(); }
float GetFontSize() { return ImGui::GetFontSize(); }
std::tuple<float, float> GetFontTexUvWhitePixel() {
    const auto vec2 {ImGui::GetFontTexUvWhitePixel()};
    return std::make_tuple(vec2.x, vec2.y);
}
#ifdef SOL_IMGUI_USE_COLOR_U32
int GetColorU32(int idx, float alphaMul) { return ImGui::GetColorU32(static_cast<ImGuiCol>(idx), alphaMul); }
int GetColorU32(float colR, float colG, float colB, float colA) { return ImGui::GetColorU32({colR, colG, colB, colA}); }
int GetColorU32(int col) { return ImGui::GetColorU32(ImU32(col)); }
#endif

// Parameters stacks (current window)
void PushItemWidth(float itemWidth) { ImGui::PushItemWidth(itemWidth); }
void PopItemWidth() { ImGui::PopItemWidth(); }
void SetNextItemWidth(float itemWidth) { ImGui::SetNextItemWidth(itemWidth); }
float CalcItemWidth() { return ImGui::CalcItemWidth(); }
void PushTextWrapPos() { ImGui::PushTextWrapPos(); }
void PushTextWrapPos(float wrapLocalPosX) { ImGui::PushTextWrapPos(wrapLocalPosX); }
void PopTextWrapPos() { ImGui::PopTextWrapPos(); }
void PushAllowKeyboardFocus(bool allowKeyboardFocus) { ImGui::PushAllowKeyboardFocus(allowKeyboardFocus); }
void PopAllowKeyboardFocus() { ImGui::PopAllowKeyboardFocus(); }
void PushButtonRepeat(bool repeat) { ImGui::PushButtonRepeat(repeat); }
void PopButtonRepeat() { ImGui::PopButtonRepeat(); }

// Cursor / Layout
void Separator() { ImGui::Separator(); }
void SameLine() { ImGui::SameLine(); }
void SameLine(float offsetFromStartX) { ImGui::SameLine(offsetFromStartX); }
void SameLine(float offsetFromStartX, float spacing) { ImGui::SameLine(offsetFromStartX, spacing); }
void NewLine() { ImGui::NewLine(); }
void Spacing() { ImGui::Spacing(); }
void Dummy(float sizeX, float sizeY) { ImGui::Dummy({sizeX, sizeY}); }
void Indent() { ImGui::Indent(); }
void Indent(float indentW) { ImGui::Indent(indentW); }
void Unindent() { ImGui::Unindent(); }
void Unindent(float indentW) { ImGui::Unindent(indentW); }
void BeginGroup() { ImGui::BeginGroup(); }
void EndGroup() { ImGui::EndGroup(); }
std::tuple<float, float> GetCursorPos() {
    const auto vec2 {ImGui::GetCursorPos()};
    return std::make_tuple(vec2.x, vec2.y);
}
float GetCursorPosX() { return ImGui::GetCursorPosX(); }
float GetCursorPosY() { return ImGui::GetCursorPosY(); }
void SetCursorPos(float localX, float localY) { ImGui::SetCursorPos({localX, localY}); }
void SetCursorPosX(float localX) { ImGui::SetCursorPosX(localX); }
void SetCursorPosY(float localY) { ImGui::SetCursorPosY(localY); }
std::tuple<float, float> GetCursorStartPos() {
    const auto vec2 {ImGui::GetCursorStartPos()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetCursorScreenPos() {
    const auto vec2 {ImGui::GetCursorScreenPos()};
    return std::make_tuple(vec2.x, vec2.y);
}
void SetCursorScreenPos(float posX, float posY) { ImGui::SetCursorScreenPos({posX, posY}); }
void AlignTextToFramePadding() { ImGui::AlignTextToFramePadding(); }
float GetTextLineHeight() { return ImGui::GetTextLineHeight(); }
float GetTextLineHeightWithSpacing() { return ImGui::GetTextLineHeightWithSpacing(); }
float GetFrameHeight() { return ImGui::GetFrameHeight(); }
float GetFrameHeightWithSpacing() { return ImGui::GetFrameHeightWithSpacing(); }

// ID stack / scopes
void PushID(const std::string& stringID) { ImGui::PushID(stringID.c_str()); }
void PushID(const std::string& stringIDBegin, const std::string& stringIDEnd) {
    ImGui::PushID(stringIDBegin.c_str(), stringIDEnd.c_str());
}
void PushID(const void* /*unused*/) { /* TODO: PushID(void*) ==> UNSUPPORTED */ }
void PushID(int intID) { ImGui::PushID(intID); }
void PopID() { ImGui::PopID(); }
int GetID(const std::string& stringID) { return ImGui::GetID(stringID.c_str()); }
int GetID(const std::string& stringIDBegin, const std::string& stringIDEnd) {
    return ImGui::GetID(stringIDBegin.c_str(), stringIDEnd.c_str());
}
int GetID(const void* /*unused*/) { return 0; /* TODO: GetID(void*) ==> UNSUPPORTED */ }

// Widgets: Text
void TextUnformatted(const std::string& text) { ImGui::TextUnformatted(text.c_str()); }
void TextUnformatted(const std::string& text, const std::string& textEnd) {
    ImGui::TextUnformatted(text.c_str(), textEnd.c_str());
}
void Text(const std::string& text) { ImGui::Text("%s", text.c_str()); }
void TextColored(float colR, float colG, float colB, float colA, const std::string& text) {
    ImGui::TextColored({colR, colG, colB, colA}, "%s", text.c_str());
}
void TextDisabled(const std::string& text) { ImGui::TextDisabled("%s", text.c_str()); }
void TextWrapped(const std::string& text) { ImGui::TextWrapped("%s", text.c_str()); }
void LabelText(const std::string& label, const std::string& text) {
    ImGui::LabelText(label.c_str(), "%s", text.c_str());
}
void BulletText(const std::string& text) { ImGui::BulletText("%s", text.c_str()); }

// Widgets: Main
bool Button(const std::string& label) { return ImGui::Button(label.c_str()); }
bool Button(const std::string& label, float sizeX, float sizeY) { return ImGui::Button(label.c_str(), {sizeX, sizeY}); }
bool SmallButton(const std::string& label) { return ImGui::SmallButton(label.c_str()); }
bool InvisibleButton(const std::string& stringID, float sizeX, float sizeY) {
    return ImGui::InvisibleButton(stringID.c_str(), {sizeX, sizeY});
}
bool ArrowButton(const std::string& stringID, int dir) {
    return ImGui::ArrowButton(stringID.c_str(), static_cast<ImGuiDir>(dir));
}
void Image() { /* TODO: Image(...) ==> UNSUPPORTED */ }
void ImageButton() { /* TODO: ImageButton(...) ==> UNSUPPORTED */ }
std::tuple<bool, bool> Checkbox(const std::string& label, bool v) {
    bool value {v};
    bool pressed = ImGui::Checkbox(label.c_str(), &value);

    return std::make_tuple(value, pressed);
}
bool CheckboxFlags() { return false; /* TODO: CheckboxFlags(...) ==> UNSUPPORTED */ }
bool RadioButton(const std::string& label, bool active) { return ImGui::RadioButton(label.c_str(), active); }
std::tuple<int, bool> RadioButton(const std::string& label, int v, int vButton) {
    bool ret {ImGui::RadioButton(label.c_str(), &v, vButton)};
    return std::make_tuple(v, ret);
}
void ProgressBar(float fraction) { ImGui::ProgressBar(fraction); }
void ProgressBar(float fraction, float sizeX, float sizeY) { ImGui::ProgressBar(fraction, {sizeX, sizeY}); }
void ProgressBar(float fraction, float sizeX, float sizeY, const std::string& overlay) {
    ImGui::ProgressBar(fraction, {sizeX, sizeY}, overlay.c_str());
}
void Bullet() { ImGui::Bullet(); }

// Widgets: Combo Box
bool BeginCombo(const std::string& label, const std::string& previewValue) {
    return ImGui::BeginCombo(label.c_str(), previewValue.c_str());
}
bool BeginCombo(const std::string& label, const std::string& previewValue, int flags) {
    return ImGui::BeginCombo(label.c_str(), previewValue.c_str(), static_cast<ImGuiComboFlags>(flags));
}
void EndCombo() { ImGui::EndCombo(); }
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const sol::table& items, int itemsCount) {
    std::vector<std::string> strings;
    for (int i {1}; i <= itemsCount; i++) {
        const auto& stringItem = items.get<sol::optional<std::string>>(i);
        strings.push_back(stringItem.value_or("Missing"));
    }

    std::vector<const char*> cstrings;
    cstrings.reserve(strings.size());
    for (auto& string : strings) cstrings.push_back(string.c_str());

    bool clicked = ImGui::Combo(label.c_str(), &currentItem, cstrings.data(), itemsCount);
    return std::make_tuple(currentItem, clicked);
}
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const sol::table& items, int itemsCount,
                            int popupMaxHeightInItems) {
    std::vector<std::string> strings;
    for (int i {1}; i <= itemsCount; i++) {
        const auto& stringItem = items.get<sol::optional<std::string>>(i);
        strings.push_back(stringItem.value_or("Missing"));
    }

    std::vector<const char*> cstrings;
    cstrings.reserve(strings.size());
    for (auto& string : strings) cstrings.push_back(string.c_str());

    bool clicked = ImGui::Combo(label.c_str(), &currentItem, cstrings.data(), itemsCount, popupMaxHeightInItems);
    return std::make_tuple(currentItem, clicked);
}
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const std::string& itemsSeparatedByZeros) {
    bool clicked = ImGui::Combo(label.c_str(), &currentItem, itemsSeparatedByZeros.c_str());
    return std::make_tuple(currentItem, clicked);
}
std::tuple<int, bool> Combo(const std::string& label, int currentItem, const std::string& itemsSeparatedByZeros,
                            int popupMaxHeightInItems) {
    bool clicked = ImGui::Combo(label.c_str(), &currentItem, itemsSeparatedByZeros.c_str(), popupMaxHeightInItems);
    return std::make_tuple(currentItem, clicked);
}
// TODO(zyunl): 3rd Combo from ImGui not Supported

// Widgets: Drags
std::tuple<float, bool> DragFloat(const std::string& label, float v) {
    bool used = ImGui::DragFloat(label.c_str(), &v);
    return std::make_tuple(v, used);
}
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed) {
    bool used = ImGui::DragFloat(label.c_str(), &v, v_speed);
    return std::make_tuple(v, used);
}
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min) {
    bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min);
    return std::make_tuple(v, used);
}
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max) {
    bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max);
    return std::make_tuple(v, used);
}
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max,
                                  const std::string& format) {
    bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max, format.c_str());
    return std::make_tuple(v, used);
}
std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max,
                                  const std::string& format, float power) {
    bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max, format.c_str(), power);
    return std::make_tuple(v, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::DragFloat2(label.c_str(), value);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::DragFloat2(label.c_str(), value, v_speed);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format, float power) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), power);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::DragFloat3(label.c_str(), value);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::DragFloat3(label.c_str(), value, v_speed);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format, float power) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), power);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::DragFloat4(label.c_str(), value);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::DragFloat4(label.c_str(), value, v_speed);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v,
                                                                 float v_speed, float v_min, float v_max,
                                                                 const std::string& format, float power) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), power);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
void DragFloatRange2() { /* TODO: DragFloatRange2(...) ==> UNSUPPORTED */ }
std::tuple<int, bool> DragInt(const std::string& label, int v) {
    bool used = ImGui::DragInt(label.c_str(), &v);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed) {
    bool used = ImGui::DragInt(label.c_str(), &v, v_speed);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min) {
    bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max) {
    bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min, v_max);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max,
                              const std::string& format) {
    bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min, v_max, format.c_str());
    return std::make_tuple(v, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::DragInt2(label.c_str(), value);

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::DragInt2(label.c_str(), value, v_speed);

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min);

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min, v_max);

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max,
                                                             const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::DragInt3(label.c_str(), value);

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::DragInt3(label.c_str(), value, v_speed);

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min);

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min, v_max);

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max,
                                                             const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::DragInt4(label.c_str(), value);

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::DragInt4(label.c_str(), value, v_speed);

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min);

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min, v_max);

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v,
                                                             float v_speed, int v_min, int v_max,
                                                             const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
void DragIntRange2() { /* TODO: DragIntRange2(...) ==> UNSUPPORTED */ }
void DragScalar() { /* TODO: DragScalar(...) ==> UNSUPPORTED */ }
void DragScalarN() { /* TODO: DragScalarN(...) ==> UNSUPPORTED */ }

// Widgets: Sliders
std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max) {
    bool used = ImGui::SliderFloat(label.c_str(), &v, v_min, v_max);
    return std::make_tuple(v, used);
}
std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max,
                                    const std::string& format) {
    bool used = ImGui::SliderFloat(label.c_str(), &v, v_min, v_max, format.c_str());
    return std::make_tuple(v, used);
}
std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max,
                                    const std::string& format, float power) {
    bool used = ImGui::SliderFloat(label.c_str(), &v, v_min, v_max, format.c_str(), power);
    return std::make_tuple(v, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max,
                                                                   const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max, format.c_str());

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format,
                                                                   float power) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max, format.c_str(), power);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[3]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max,
                                                                   const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max, format.c_str());

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[3]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format,
                                                                   float power) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max, format.c_str(), power);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[3]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max,
                                                                   const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max, format.c_str());

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v,
                                                                   float v_min, float v_max, const std::string& format,
                                                                   float power) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max, format.c_str(), power);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad) {
    bool used = ImGui::SliderAngle(label.c_str(), &v_rad);
    return std::make_tuple(v_rad, used);
}
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min) {
    bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min);
    return std::make_tuple(v_rad, used);
}
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max) {
    bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min, v_degrees_max);
    return std::make_tuple(v_rad, used);
}
std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max,
                                    const std::string& format) {
    bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min, v_degrees_max, format.c_str());
    return std::make_tuple(v_rad, used);
}
std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max) {
    bool used = ImGui::SliderInt(label.c_str(), &v, v_min, v_max);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max, const std::string& format) {
    bool used = ImGui::SliderInt(label.c_str(), &v, v_min, v_max, format.c_str());
    return std::make_tuple(v, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::SliderInt2(label.c_str(), value, v_min, v_max);

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max, const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::SliderInt2(label.c_str(), value, v_min, v_max, format.c_str());

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::SliderInt3(label.c_str(), value, v_min, v_max);

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max, const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::SliderInt3(label.c_str(), value, v_min, v_max, format.c_str());

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::SliderInt4(label.c_str(), value, v_min, v_max);

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min,
                                                               int v_max, const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::SliderInt4(label.c_str(), value, v_min, v_max, format.c_str());

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
void SliderScalar() { /* TODO: SliderScalar(...) ==> UNSUPPORTED */ }
void SliderScalarN() { /* TODO: SliderScalarN(...) ==> UNSUPPORTED */ }
std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min,
                                     float v_max) {
    bool used = ImGui::VSliderFloat(label.c_str(), {sizeX, sizeY}, &v, v_min, v_max);
    return std::make_tuple(v, used);
}
std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min,
                                     float v_max, const std::string& format) {
    bool used = ImGui::VSliderFloat(label.c_str(), {sizeX, sizeY}, &v, v_min, v_max, format.c_str());
    return std::make_tuple(v, used);
}
std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min,
                                     float v_max, const std::string& format, float power) {
    bool used = ImGui::VSliderFloat(label.c_str(), {sizeX, sizeY}, &v, v_min, v_max, format.c_str(), power);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max) {
    bool used = ImGui::VSliderInt(label.c_str(), {sizeX, sizeY}, &v, v_min, v_max);
    return std::make_tuple(v, used);
}
std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max,
                                 const std::string& format) {
    bool used = ImGui::VSliderInt(label.c_str(), {sizeX, sizeY}, &v, v_min, v_max, format.c_str());
    return std::make_tuple(v, used);
}
void VSliderScalar() { /* TODO: VSliderScalar(...) ==> UNSUPPORTED */ }

// Widgets: Input with Keyboard
std::tuple<std::string, bool> InputText(const std::string& label, std::string text, unsigned int buf_size) {
    bool selected = ImGui::InputText(label.c_str(), text.data(), buf_size);
    return std::make_tuple(text, selected);
}
std::tuple<std::string, bool> InputText(const std::string& label, std::string text, unsigned int buf_size, int flags) {
    bool selected = ImGui::InputText(label.c_str(), text.data(), buf_size, static_cast<ImGuiInputTextFlags>(flags));
    return std::make_tuple(text, selected);
}
std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size) {
    bool selected = ImGui::InputTextMultiline(label.c_str(), text.data(), buf_size);
    return std::make_tuple(text, selected);
}
std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size,
                                                 float sizeX, float sizeY) {
    bool selected = ImGui::InputTextMultiline(label.c_str(), text.data(), buf_size, {sizeX, sizeY});
    return std::make_tuple(text, selected);
}
std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size,
                                                 float sizeX, float sizeY, int flags) {
    bool selected = ImGui::InputTextMultiline(label.c_str(), text.data(), buf_size, {sizeX, sizeY},
                                              static_cast<ImGuiInputTextFlags>(flags));
    return std::make_tuple(text, selected);
}
std::tuple<std::string, bool> InputTextWithHint(const std::string& label, const std::string& hint, std::string text,
                                                unsigned int buf_size) {
    bool selected = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), text.data(), buf_size);
    return std::make_tuple(text, selected);
}
std::tuple<std::string, bool> InputTextWithHint(const std::string& label, const std::string& hint, std::string text,
                                                unsigned int buf_size, int flags) {
    bool selected = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), text.data(), buf_size,
                                             static_cast<ImGuiInputTextFlags>(flags));
    return std::make_tuple(text, selected);
}
std::tuple<float, bool> InputFloat(const std::string& label, float v) {
    bool selected = ImGui::InputFloat(label.c_str(), &v);
    return std::make_tuple(v, selected);
}
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step) {
    bool selected = ImGui::InputFloat(label.c_str(), &v, step);
    return std::make_tuple(v, selected);
}
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast) {
    bool selected = ImGui::InputFloat(label.c_str(), &v, step, step_fast);
    return std::make_tuple(v, selected);
}
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast,
                                   const std::string& format) {
    bool selected = ImGui::InputFloat(label.c_str(), &v, step, step_fast, format.c_str());
    return std::make_tuple(v, selected);
}
std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast,
                                   const std::string& format, int flags) {
    bool selected =
        ImGui::InputFloat(label.c_str(), &v, step, step_fast, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));
    return std::make_tuple(v, selected);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::InputFloat2(label.c_str(), value);

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v,
                                                                  const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::InputFloat2(label.c_str(), value, format.c_str());

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v,
                                                                  const std::string& format, int flags) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[2] = {float(v1), float(v2)};
    bool used = ImGui::InputFloat2(label.c_str(), value, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));

    sol::as_table_t float2 = sol::as_table(std::vector<float> {value[0], value[1]});

    return std::make_tuple(float2, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::InputFloat3(label.c_str(), value);

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v,
                                                                  const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::InputFloat3(label.c_str(), value, format.c_str());

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v,
                                                                  const std::string& format, int flags) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[3] = {float(v1), float(v2), float(v3)};
    bool used = ImGui::InputFloat3(label.c_str(), value, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));

    sol::as_table_t float3 = sol::as_table(std::vector<float> {value[0], value[1], value[2]});

    return std::make_tuple(float3, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::InputFloat4(label.c_str(), value);

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v,
                                                                  const std::string& format) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::InputFloat4(label.c_str(), value, format.c_str());

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v,
                                                                  const std::string& format, int flags) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float value[4] = {float(v1), float(v2), float(v3), float(v4)};
    bool used = ImGui::InputFloat4(label.c_str(), value, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));

    sol::as_table_t float4 = sol::as_table(std::vector<float> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(float4, used);
}
std::tuple<int, bool> InputInt(const std::string& label, int v) {
    bool selected = ImGui::InputInt(label.c_str(), &v);
    return std::make_tuple(v, selected);
}
std::tuple<int, bool> InputInt(const std::string& label, int v, int step) {
    bool selected = ImGui::InputInt(label.c_str(), &v, step);
    return std::make_tuple(v, selected);
}
std::tuple<int, bool> InputInt(const std::string& label, int v, int step, int step_fast) {
    bool selected = ImGui::InputInt(label.c_str(), &v, step, step_fast);
    return std::make_tuple(v, selected);
}
std::tuple<int, bool> InputInt(const std::string& label, int v, int step, int step_fast, int flags) {
    bool selected = ImGui::InputInt(label.c_str(), &v, step, step_fast, static_cast<ImGuiInputTextFlags>(flags));
    return std::make_tuple(v, selected);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt2(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::InputInt2(label.c_str(), value);

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt2(const std::string& label, const sol::table& v,
                                                              int flags) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[2] = {int(v1), int(v2)};
    bool used = ImGui::InputInt2(label.c_str(), value, static_cast<ImGuiInputTextFlags>(flags));

    sol::as_table_t int2 = sol::as_table(std::vector<int> {value[0], value[1]});

    return std::make_tuple(int2, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt3(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::InputInt3(label.c_str(), value);

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt3(const std::string& label, const sol::table& v,
                                                              int flags) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[3] = {int(v1), int(v2), int(v3)};
    bool used = ImGui::InputInt3(label.c_str(), value, static_cast<ImGuiInputTextFlags>(flags));

    sol::as_table_t int3 = sol::as_table(std::vector<int> {value[0], value[1], value[2]});

    return std::make_tuple(int3, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt4(const std::string& label, const sol::table& v) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::InputInt4(label.c_str(), value);

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<sol::as_table_t<std::vector<int>>, bool> InputInt4(const std::string& label, const sol::table& v,
                                                              int flags) {
    const lua_Number v1 {v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v2 {v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v3 {v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number v4 {v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    int value[4] = {int(v1), int(v2), int(v3), int(v4)};
    bool used = ImGui::InputInt4(label.c_str(), value, static_cast<ImGuiInputTextFlags>(flags));

    sol::as_table_t int4 = sol::as_table(std::vector<int> {value[0], value[1], value[2], value[3]});

    return std::make_tuple(int4, used);
}
std::tuple<double, bool> InputDouble(const std::string& label, double v) {
    bool selected = ImGui::InputDouble(label.c_str(), &v);
    return std::make_tuple(v, selected);
}
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step) {
    bool selected = ImGui::InputDouble(label.c_str(), &v, step);
    return std::make_tuple(v, selected);
}
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast) {
    bool selected = ImGui::InputDouble(label.c_str(), &v, step, step_fast);
    return std::make_tuple(v, selected);
}
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast,
                                     const std::string& format) {
    bool selected = ImGui::InputDouble(label.c_str(), &v, step, step_fast, format.c_str());
    return std::make_tuple(v, selected);
}
std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast,
                                     const std::string& format, int flags) {
    bool selected =
        ImGui::InputDouble(label.c_str(), &v, step, step_fast, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));
    return std::make_tuple(v, selected);
}
void InputScalar() { /* TODO: InputScalar(...) ==> UNSUPPORTED */ }
void InputScalarN() { /* TODO: InputScalarN(...) ==> UNSUPPORTED */ }

// Widgets: Color Editor / Picker
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[3] = {float(r), float(g), float(b)};
    bool used = ImGui::ColorEdit3(label.c_str(), color);

    sol::as_table_t rgb = sol::as_table(std::vector<float> {color[0], color[1], color[2]});

    return std::make_tuple(rgb, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col,
                                                                 int flags) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[3] = {float(r), float(g), float(b)};
    bool used = ImGui::ColorEdit3(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

    sol::as_table_t rgb = sol::as_table(std::vector<float> {color[0], color[1], color[2]});

    return std::make_tuple(rgb, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[4] = {float(r), float(g), float(b), float(a)};
    bool used = ImGui::ColorEdit4(label.c_str(), color);

    sol::as_table_t rgba = sol::as_table(std::vector<float> {color[0], color[1], color[2], color[3]});

    return std::make_tuple(rgba, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col,
                                                                 int flags) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[4] = {float(r), float(g), float(b), float(a)};
    bool used = ImGui::ColorEdit4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

    sol::as_table_t rgba = sol::as_table(std::vector<float> {color[0], color[1], color[2], color[3]});

    return std::make_tuple(rgba, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[3] = {float(r), float(g), float(b)};
    bool used = ImGui::ColorPicker3(label.c_str(), color);

    sol::as_table_t rgb = sol::as_table(std::vector<float> {color[0], color[1], color[2]});

    return std::make_tuple(rgb, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col,
                                                                   int flags) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[3] = {float(r), float(g), float(b)};
    bool used = ImGui::ColorPicker3(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

    sol::as_table_t rgb = sol::as_table(std::vector<float> {color[0], color[1], color[2]});

    return std::make_tuple(rgb, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[4] = {float(r), float(g), float(b), float(a)};
    bool used = ImGui::ColorPicker4(label.c_str(), color);

    sol::as_table_t rgba = sol::as_table(std::vector<float> {color[0], color[1], color[2], color[3]});

    return std::make_tuple(rgba, used);
}
std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col,
                                                                   int flags) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    float color[4] = {float(r), float(g), float(b), float(a)};
    bool used = ImGui::ColorPicker4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

    sol::as_table_t rgba = sol::as_table(std::vector<float> {color[0], color[1], color[2], color[3]});

    return std::make_tuple(rgba, used);
}
bool ColorButton(const std::string& desc_id, const sol::table& col) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const ImVec4 color {float(r), float(g), float(b), float(a)};
    return ImGui::ColorButton(desc_id.c_str(), color);
}
bool ColorButton(const std::string& desc_id, const sol::table& col, int flags) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const ImVec4 color {float(r), float(g), float(b), float(a)};
    return ImGui::ColorButton(desc_id.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));
}
bool ColorButton(const std::string& desc_id, const sol::table& col, int flags, float sizeX, float sizeY) {
    const lua_Number r {col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number g {col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number b {col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const lua_Number a {col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};
    const ImVec4 color {float(r), float(g), float(b), float(a)};
    return ImGui::ColorButton(desc_id.c_str(), color, static_cast<ImGuiColorEditFlags>(flags), {sizeX, sizeY});
}
void SetColorEditOptions(int flags) { ImGui::SetColorEditOptions(static_cast<ImGuiColorEditFlags>(flags)); }

// Widgets: Trees
bool TreeNode(const std::string& label) { return ImGui::TreeNode(label.c_str()); }
bool TreeNode(const std::string& label, const std::string& fmt) {
    return ImGui::TreeNode(label.c_str(), "%s", fmt.c_str());
}
/* TODO: TreeNodeV(...) (2) ==> UNSUPPORTED */
bool TreeNodeEx(const std::string& label) { return ImGui::TreeNodeEx(label.c_str()); }
bool TreeNodeEx(const std::string& label, int flags) {
    return ImGui::TreeNodeEx(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags));
}
bool TreeNodeEx(const std::string& label, int flags, const std::string& fmt) {
    return ImGui::TreeNodeEx(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags), "%s", fmt.c_str());
}
/* TODO: TreeNodeExV(...) (2) ==> UNSUPPORTED */
void TreePush(const std::string& str_id) { ImGui::TreePush(str_id.c_str()); }
/* TODO: TreePush(const void*) ==> UNSUPPORTED */
void TreePop() { ImGui::TreePop(); }
float GetTreeNodeToLabelSpacing() { return ImGui::GetTreeNodeToLabelSpacing(); }
bool CollapsingHeader(const std::string& label) { return ImGui::CollapsingHeader(label.c_str()); }
bool CollapsingHeader(const std::string& label, int flags) {
    return ImGui::CollapsingHeader(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags));
}
std::tuple<bool, bool> CollapsingHeader(const std::string& label, bool open) {
    bool notCollapsed = ImGui::CollapsingHeader(label.c_str(), &open);
    return std::make_tuple(open, notCollapsed);
}
std::tuple<bool, bool> CollapsingHeader(const std::string& label, bool open, int flags) {
    bool notCollapsed = ImGui::CollapsingHeader(label.c_str(), &open, static_cast<ImGuiTreeNodeFlags>(flags));
    return std::make_tuple(open, notCollapsed);
}
void SetNextItemOpen(bool is_open) { ImGui::SetNextItemOpen(is_open); }
void SetNextItemOpen(bool is_open, int cond) { ImGui::SetNextItemOpen(is_open, static_cast<ImGuiCond>(cond)); }

// Widgets: Selectables
// TODO(zyunl): Only one of Selectable variations is possible due to same parameters for Lua
bool Selectable(const std::string& label) { return ImGui::Selectable(label.c_str()); }
bool Selectable(const std::string& label, bool selected) {
    ImGui::Selectable(label.c_str(), &selected);
    return selected;
}
bool Selectable(const std::string& label, bool selected, int flags) {
    ImGui::Selectable(label.c_str(), &selected, static_cast<ImGuiSelectableFlags>(flags));
    return selected;
}
bool Selectable(const std::string& label, bool selected, int flags, float sizeX, float sizeY) {
    ImGui::Selectable(label.c_str(), &selected, static_cast<ImGuiSelectableFlags>(flags), {sizeX, sizeY});
    return selected;
}

// Widgets: List Boxes
std::tuple<int, bool> ListBox(const std::string& label, int current_item, const sol::table& items, int items_count) {
    std::vector<std::string> strings;
    for (int i {1}; i <= items_count; i++) {
        const auto& stringItem = items.get<sol::optional<std::string>>(i);
        strings.push_back(stringItem.value_or("Missing"));
    }

    std::vector<const char*> cstrings;
    cstrings.reserve(strings.size());
    for (auto& string : strings) cstrings.push_back(string.c_str());

    bool clicked = ImGui::ListBox(label.c_str(), &current_item, cstrings.data(), items_count);
    return std::make_tuple(current_item, clicked);
}
std::tuple<int, bool> ListBox(const std::string& label, int current_item, const sol::table& items, int items_count,
                              int height_in_items) {
    std::vector<std::string> strings;
    for (int i {1}; i <= items_count; i++) {
        const auto& stringItem = items.get<sol::optional<std::string>>(i);
        strings.push_back(stringItem.value_or("Missing"));
    }

    std::vector<const char*> cstrings;
    cstrings.reserve(strings.size());
    for (auto& string : strings) cstrings.push_back(string.c_str());

    bool clicked = ImGui::ListBox(label.c_str(), &current_item, cstrings.data(), items_count, height_in_items);
    return std::make_tuple(current_item, clicked);
}
bool ListBoxHeader(const std::string& label, float sizeX, float sizeY) {
    return ImGui::ListBoxHeader(label.c_str(), {sizeX, sizeY});
}
bool ListBoxHeader(const std::string& label, int items_count) {
    return ImGui::ListBoxHeader(label.c_str(), items_count);
}
bool ListBoxHeader(const std::string& label, int items_count, int height_in_items) {
    return ImGui::ListBoxHeader(label.c_str(), items_count, height_in_items);
}
void ListBoxFooter() { ImGui::ListBoxFooter(); }

// Widgets: Data Plotting
/* TODO: Widgets Data Plotting ==> UNSUPPORTED (barely used and quite long functions) */

// Widgets: Value() helpers
void Value(const std::string& prefix, bool b) { ImGui::Value(prefix.c_str(), b); }
void Value(const std::string& prefix, int v) { ImGui::Value(prefix.c_str(), v); }
void Value(const std::string& prefix, unsigned int v) { ImGui::Value(prefix.c_str(), v); }
void Value(const std::string& prefix, float v) { ImGui::Value(prefix.c_str(), v); }
void Value(const std::string& prefix, float v, const std::string& float_format) {
    ImGui::Value(prefix.c_str(), v, float_format.c_str());
}

// Widgets: Menus
bool BeginMenuBar() { return ImGui::BeginMenuBar(); }
void EndMenuBar() { ImGui::EndMenuBar(); }
bool BeginMainMenuBar() { return ImGui::BeginMainMenuBar(); }
void EndMainMenuBar() { ImGui::EndMainMenuBar(); }
bool BeginMenu(const std::string& label) { return ImGui::BeginMenu(label.c_str()); }
bool BeginMenu(const std::string& label, bool enabled) { return ImGui::BeginMenu(label.c_str(), enabled); }
void EndMenu() { ImGui::EndMenu(); }
bool MenuItem(const std::string& label) { return ImGui::MenuItem(label.c_str()); }
bool MenuItem(const std::string& label, const std::string& shortcut) {
    return ImGui::MenuItem(label.c_str(), shortcut.c_str());
}
std::tuple<bool, bool> MenuItem(const std::string& label, const std::string& shortcut, bool selected) {
    bool activated = ImGui::MenuItem(label.c_str(), shortcut.c_str(), &selected);
    return std::make_tuple(selected, activated);
}
std::tuple<bool, bool> MenuItem(const std::string& label, const std::string& shortcut, bool selected, bool enabled) {
    bool activated = ImGui::MenuItem(label.c_str(), shortcut.c_str(), &selected, enabled);
    return std::make_tuple(selected, activated);
}

// Tooltips
void BeginTooltip() { ImGui::BeginTooltip(); }
void EndTooltip() { ImGui::EndTooltip(); }
void SetTooltip(const std::string& fmt) { ImGui::SetTooltip("%s", fmt.c_str()); }
void SetTooltipV() { /* TODO: SetTooltipV(...) ==> UNSUPPORTED */ }

// Popups, Modals
bool BeginPopup(const std::string& str_id) { return ImGui::BeginPopup(str_id.c_str()); }
bool BeginPopup(const std::string& str_id, int flags) {
    return ImGui::BeginPopup(str_id.c_str(), static_cast<ImGuiWindowFlags>(flags));
}
bool BeginPopupModal(const std::string& name) { return ImGui::BeginPopupModal(name.c_str()); }
bool BeginPopupModal(const std::string& name, bool open) { return ImGui::BeginPopupModal(name.c_str(), &open); }
bool BeginPopupModal(const std::string& name, bool open, int flags) {
    return ImGui::BeginPopupModal(name.c_str(), &open, static_cast<ImGuiWindowFlags>(flags));
}
void EndPopup() { ImGui::EndPopup(); }
void OpenPopup(const std::string& str_id) { ImGui::OpenPopup(str_id.c_str()); }
void OpenPopup(const std::string& str_id, int popup_flags) {
    ImGui::OpenPopup(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags));
}
void OpenPopupOnItemClick() { ImGui::OpenPopupOnItemClick(); }  // TODO(zyunl): Verify
void OpenPopupContextItem(const std::string& str_id) { ImGui::OpenPopupOnItemClick(str_id.c_str()); }
bool OpenPopupContextItem(const std::string& str_id, int popup_flags) {
    return ImGui::BeginPopupContextItem(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags));
}
void CloseCurrentPopup() { ImGui::CloseCurrentPopup(); }
bool BeginPopupContextItem() { return ImGui::BeginPopupContextItem(); }
bool BeginPopupContextItem(const std::string& str_id) { return ImGui::BeginPopupContextItem(str_id.c_str()); }
bool BeginPopupContextItem(const std::string& str_id, int popup_flags) {
    return ImGui::BeginPopupContextItem(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags));
}
bool BeginPopupContextWindow() { return ImGui::BeginPopupContextWindow(); }
bool BeginPopupContextWindow(const std::string& str_id) { return ImGui::BeginPopupContextWindow(str_id.c_str()); }
bool BeginPopupContextWindow(const std::string& str_id, int popup_flags) {
    return ImGui::BeginPopupContextWindow(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags));
}
bool BeginPopupContextVoid() { return ImGui::BeginPopupContextVoid(); }
bool BeginPopupContextVoid(const std::string& str_id) { return ImGui::BeginPopupContextVoid(str_id.c_str()); }
bool BeginPopupContextVoid(const std::string& str_id, int popup_flags) {
    return ImGui::BeginPopupContextVoid(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags));
}
bool IsPopupOpen(const std::string& str_id) { return ImGui::IsPopupOpen(str_id.c_str()); }
bool IsPopupOpen(const std::string& str_id, int popup_flags) { return ImGui::IsPopupOpen(str_id.c_str(), popup_flags); }

// Columns
void Columns() { ImGui::Columns(); }
void Columns(int count) { ImGui::Columns(count); }
void Columns(int count, const std::string& id) { ImGui::Columns(count, id.c_str()); }
void Columns(int count, const std::string& id, bool border) { ImGui::Columns(count, id.c_str(), border); }
void NextColumn() { ImGui::NextColumn(); }
int GetColumnIndex() { return ImGui::GetColumnIndex(); }
float GetColumnWidth() { return ImGui::GetColumnWidth(); }
float GetColumnWidth(int column_index) { return ImGui::GetColumnWidth(column_index); }
void SetColumnWidth(int column_index, float width) { ImGui::SetColumnWidth(column_index, width); }
float GetColumnOffset() { return ImGui::GetColumnOffset(); }
float GetColumnOffset(int column_index) { return ImGui::GetColumnOffset(column_index); }
void SetColumnOffset(int column_index, float offset_x) { ImGui::SetColumnOffset(column_index, offset_x); }
int GetColumnsCount() { return ImGui::GetColumnsCount(); }

// Tab Bars, Tabs
bool BeginTabBar(const std::string& str_id) { return ImGui::BeginTabBar(str_id.c_str()); }
bool BeginTabBar(const std::string& str_id, int flags) {
    return ImGui::BeginTabBar(str_id.c_str(), static_cast<ImGuiTabBarFlags>(flags));
}
void EndTabBar() { ImGui::EndTabBar(); }
bool BeginTabItem(const std::string& label) { return ImGui::BeginTabItem(label.c_str()); }
std::tuple<bool, bool> BeginTabItem(const std::string& label, bool open) {
    bool selected = ImGui::BeginTabItem(label.c_str(), &open);
    return std::make_tuple(open, selected);
}
std::tuple<bool, bool> BeginTabItem(const std::string& label, bool open, int flags) {
    bool selected = ImGui::BeginTabItem(label.c_str(), &open, static_cast<ImGuiTabItemFlags>(flags));
    return std::make_tuple(open, selected);
}
void EndTabItem() { ImGui::EndTabItem(); }
void SetTabItemClosed(const std::string& tab_or_docked_window_label) {
    ImGui::SetTabItemClosed(tab_or_docked_window_label.c_str());
}
#ifdef IMGUI_HAS_DOCK
// Docking
void DockSpace(unsigned int id) { ImGui::DockSpace(id); }
void DockSpace(unsigned int id, float sizeX, float sizeY) { ImGui::DockSpace(id, {sizeX, sizeY}); }
void DockSpace(unsigned int id, float sizeX, float sizeY, int flags) {
    ImGui::DockSpace(id, {sizeX, sizeY}, static_cast<ImGuiDockNodeFlags>(flags));
}
unsigned int DockSpaceOverViewport() { return 0; /* TODO: DockSpaceOverViwport(...) ==> UNSUPPORTED */ }
void SetNextWindowDockID(unsigned int dock_id) { ImGui::SetNextWindowDockID(dock_id); }
void SetNextWindowDockID(unsigned int dock_id, int cond) {
    ImGui::SetNextWindowDockID(dock_id, static_cast<ImGuiCond>(cond));
}
void SetNextWindowClass() { /* TODO: SetNextWindowClass(...) ==> UNSUPPORTED */ }
unsigned int GetWindowDockID() { return ImGui::GetWindowDockID(); }
bool IsWindowDocked() { return ImGui::IsWindowDocked(); }
#endif  // IMGUI_HAS_DOC
// Logging
void LogToTTY() { ImGui::LogToTTY(); }
void LogToTTY(int auto_open_depth) { ImGui::LogToTTY(auto_open_depth); }
void LogToFile() { ImGui::LogToFile(); }
void LogToFile(int auto_open_depth) { ImGui::LogToFile(auto_open_depth); }
void LogToFile(int auto_open_depth, const std::string& filename) {
    ImGui::LogToFile(auto_open_depth, filename.c_str());
}
void LogToClipboard() { ImGui::LogToClipboard(); }
void LogToClipboard(int auto_open_depth) { ImGui::LogToClipboard(auto_open_depth); }
void LogFinish() { ImGui::LogFinish(); }
void LogButtons() { ImGui::LogButtons(); }
void LogText(const std::string& fmt) { ImGui::LogText("%s", fmt.c_str()); }

// Drag and Drop
// TODO(zyunl): Drag and Drop ==> UNSUPPORTED

// Clipping
void PushClipRect(float min_x, float min_y, float max_x, float max_y, bool intersect_current) {
    ImGui::PushClipRect({min_x, min_y}, {max_x, max_y}, intersect_current);
}
void PopClipRect() { ImGui::PopClipRect(); }

// Focus, Activation
void SetItemDefaultFocus() { ImGui::SetItemDefaultFocus(); }
void SetKeyboardFocusHere() { ImGui::SetKeyboardFocusHere(); }
void SetKeyboardFocusHere(int offset) { ImGui::SetKeyboardFocusHere(offset); }

// Item/Widgets Utilities
bool IsItemHovered() { return ImGui::IsItemHovered(); }
bool IsItemHovered(int flags) { return ImGui::IsItemHovered(static_cast<ImGuiHoveredFlags>(flags)); }
bool IsItemActive() { return ImGui::IsItemActive(); }
bool IsItemFocused() { return ImGui::IsItemFocused(); }
bool IsItemClicked() { return ImGui::IsItemClicked(); }
bool IsItemClicked(int mouse_button) { return ImGui::IsItemClicked(static_cast<ImGuiMouseButton>(mouse_button)); }
bool IsItemVisible() { return ImGui::IsItemVisible(); }
bool IsItemEdited() { return ImGui::IsItemEdited(); }
bool IsItemActivated() { return ImGui::IsItemActivated(); }
bool IsItemDeactivated() { return ImGui::IsItemDeactivated(); }
bool IsItemDeactivatedAfterEdit() { return ImGui::IsItemDeactivatedAfterEdit(); }
bool IsItemToggledOpen() { return ImGui::IsItemToggledOpen(); }
bool IsAnyItemHovered() { return ImGui::IsAnyItemHovered(); }
bool IsAnyItemActive() { return ImGui::IsAnyItemActive(); }
bool IsAnyItemFocused() { return ImGui::IsAnyItemFocused(); }
std::tuple<float, float> GetItemRectMin() {
    const auto vec2 {ImGui::GetItemRectMin()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetItemRectMax() {
    const auto vec2 {ImGui::GetItemRectMax()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetItemRectSize() {
    const auto vec2 {ImGui::GetItemRectSize()};
    return std::make_tuple(vec2.x, vec2.y);
}
void SetItemAllowOverlap() { ImGui::SetItemAllowOverlap(); }

// Miscellaneous Utilities
bool IsRectVisible(float sizeX, float sizeY) { return ImGui::IsRectVisible({sizeX, sizeY}); }
bool IsRectVisible(float minX, float minY, float maxX, float maxY) {
    return ImGui::IsRectVisible({minX, minY}, {maxX, maxY});
}
double GetTime() { return ImGui::GetTime(); }
int GetFrameCount() { return ImGui::GetFrameCount(); }
/* TODO: GetBackgroundDrawList(), GetForeGroundDrawList(), GetDrawListSharedData() ==> UNSUPPORTED */
std::string GetStyleColorName(int idx) { return std::string(ImGui::GetStyleColorName(static_cast<ImGuiCol>(idx))); }
/* TODO: SetStateStorage(), GetStateStorage(), CalcListClipping() ==> UNSUPPORTED */
bool BeginChildFrame(unsigned int id, float sizeX, float sizeY) { return ImGui::BeginChildFrame(id, {sizeX, sizeY}); }
bool BeginChildFrame(unsigned int id, float sizeX, float sizeY, int flags) {
    return ImGui::BeginChildFrame(id, {sizeX, sizeY}, static_cast<ImGuiWindowFlags>(flags));
}
void EndChildFrame() { ImGui::EndChildFrame(); }

// Text Utilities
std::tuple<float, float> CalcTextSize(const std::string& text) {
    const auto vec2 {ImGui::CalcTextSize(text.c_str())};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> CalcTextSize(const std::string& text, const std::string& text_end) {
    const auto vec2 {ImGui::CalcTextSize(text.c_str(), text_end.c_str())};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> CalcTextSize(const std::string& text, const std::string& text_end,
                                      bool hide_text_after_double_hash) {
    const auto vec2 {ImGui::CalcTextSize(text.c_str(), text_end.c_str(), hide_text_after_double_hash)};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> CalcTextSize(const std::string& text, const std::string& text_end,
                                      bool hide_text_after_double_hash, float wrap_width) {
    const auto vec2 {ImGui::CalcTextSize(text.c_str(), text_end.c_str(), hide_text_after_double_hash, wrap_width)};
    return std::make_tuple(vec2.x, vec2.y);
}

// Color Utilities
#ifdef SOL_IMGUI_USE_COLOR_U32
sol::as_table_t<std::vector<float>> ColorConvertU32ToFloat4(unsigned int in) {
    const auto vec4 = ImGui::ColorConvertU32ToFloat4(in);
    sol::as_table_t rgba = sol::as_table(std::vector<float> {vec4.x, vec4.y, vec4.z, vec4.w});

    return rgba;
}
unsigned int ColorConvertFloat4ToU32(const sol::table& rgba) {
    const lua_Number r {rgba[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))},
        g {rgba[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))},
        b {rgba[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))},
        a {rgba[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0))};

    return ImGui::ColorConvertFloat4ToU32({float(r), float(g), float(b), float(a)});
}
#endif
std::tuple<float, float, float> ColorConvertRGBtoHSV(float r, float g, float b) {
    float h {};
    float s {};
    float v {};
    ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);
    return std::make_tuple(h, s, v);
}
std::tuple<float, float, float> ColorConvertHSVtoRGB(float h, float s, float v) {
    float r {};
    float g {};
    float b {};
    ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
    return std::make_tuple(r, g, b);
}

// Inputs Utilities: Keyboard
int GetKeyIndex(int imgui_key) { return ImGui::GetKeyIndex(static_cast<ImGuiKey>(imgui_key)); }
bool IsKeyDown(int user_key_index) { return ImGui::IsKeyDown(user_key_index); }
bool IsKeyPressed(int user_key_index) { return ImGui::IsKeyPressed(user_key_index); }
bool IsKeyPressed(int user_key_index, bool repeat) { return ImGui::IsKeyPressed(user_key_index, repeat); }
bool IsKeyReleased(int user_key_index) { return ImGui::IsKeyReleased(user_key_index); }
int GetKeyPressedAmount(int key_index, float repeat_delay, float rate) {
    return ImGui::GetKeyPressedAmount(key_index, repeat_delay, rate);
}
void CaptureKeyboardFromApp() { ImGui::CaptureKeyboardFromApp(); }
void CaptureKeyboardFromApp(bool want_capture_keyboard_value) {
    ImGui::CaptureKeyboardFromApp(want_capture_keyboard_value);
}

// Inputs Utilities: Mouse
bool IsMouseDown(int button) { return ImGui::IsMouseDown(static_cast<ImGuiMouseButton>(button)); }
bool IsMouseClicked(int button) { return ImGui::IsMouseClicked(static_cast<ImGuiMouseButton>(button)); }
bool IsMouseClicked(int button, bool repeat) {
    return ImGui::IsMouseClicked(static_cast<ImGuiMouseButton>(button), repeat);
}
bool IsMouseReleased(int button) { return ImGui::IsMouseReleased(static_cast<ImGuiMouseButton>(button)); }
bool IsMouseDoubleClicked(int button) { return ImGui::IsMouseDoubleClicked(static_cast<ImGuiMouseButton>(button)); }
bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y) {
    return ImGui::IsMouseHoveringRect({min_x, min_y}, {max_x, max_y});
}
bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y, bool clip) {
    return ImGui::IsMouseHoveringRect({min_x, min_y}, {max_x, max_y}, clip);
}
bool IsMousePosValid() { return false; /* TODO: IsMousePosValid() ==> UNSUPPORTED */ }
bool IsAnyMouseDown() { return ImGui::IsAnyMouseDown(); }
std::tuple<float, float> GetMousePos() {
    const auto vec2 {ImGui::GetMousePos()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetMousePosOnOpeningCurrentPopup() {
    const auto vec2 {ImGui::GetMousePosOnOpeningCurrentPopup()};
    return std::make_tuple(vec2.x, vec2.y);
}
bool IsMouseDragging(int button) { return ImGui::IsMouseDragging(static_cast<ImGuiMouseButton>(button)); }
bool IsMouseDragging(int button, float lock_threshold) {
    return ImGui::IsMouseDragging(static_cast<ImGuiMouseButton>(button), lock_threshold);
}
std::tuple<float, float> GetMouseDragDelta() {
    const auto vec2 {ImGui::GetMouseDragDelta()};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetMouseDragDelta(int button) {
    const auto vec2 {ImGui::GetMouseDragDelta(static_cast<ImGuiMouseButton>(button))};
    return std::make_tuple(vec2.x, vec2.y);
}
std::tuple<float, float> GetMouseDragDelta(int button, float lock_threshold) {
    const auto vec2 {ImGui::GetMouseDragDelta(static_cast<ImGuiMouseButton>(button), lock_threshold)};
    return std::make_tuple(vec2.x, vec2.y);
}
void ResetMouseDragDelta() { ImGui::ResetMouseDragDelta(); }
void ResetMouseDragDelta(int button) { ImGui::ResetMouseDragDelta(static_cast<ImGuiMouseButton>(button)); }
int GetMouseCursor() { return ImGui::GetMouseCursor(); }
void SetMouseCursor(int cursor_type) { ImGui::SetMouseCursor(static_cast<ImGuiMouseCursor>(cursor_type)); }
void CaptureMouseFromApp() { ImGui::CaptureMouseFromApp(); }
void CaptureMouseFromApp(bool want_capture_mouse_value) { ImGui::CaptureMouseFromApp(want_capture_mouse_value); }

// Clipboard Utilities
std::string GetClipboardText() { return std::string(ImGui::GetClipboardText()); }
void SetClipboardText(const std::string& text) { ImGui::SetClipboardText(text.c_str()); }

bool BeginTable(const char* name, int columns) { return ImGui::BeginTable(name, columns); }

void EndTable() { ImGui::EndTable(); }

void TableNextRow() { ImGui::TableNextRow(); }

void TableNextRow(ImGuiTableRowFlags row_flags = 0) { ImGui::TableNextRow(row_flags); }

void TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f) {
    ImGui::TableNextRow(row_flags, min_row_height);
}

bool TableNextColumn() { return ImGui::TableNextColumn(); }

void TableSetupColumn(const char* label) { ImGui::TableSetupColumn(label); }

void TableSetupColumn(const char* label, ImGuiTableColumnFlags flag) { ImGui::TableSetupColumn(label, flag); }

void TableSetupColumn(const char* label, ImGuiTableColumnFlags flag, float init_width_or_weight) {
    ImGui::TableSetupColumn(label, flag, init_width_or_weight);
}

void TableSetupColumn(const char* label, ImGuiTableColumnFlags flag, float init_width_or_weight, ImGuiID user_id) {
    ImGui::TableSetupColumn(label, flag, init_width_or_weight, user_id);
}

bool TableSetColumnIndex(int column_n) { return ImGui::TableSetColumnIndex(column_n); }

void TableSetupScrollFreeze(int cols, int rows) { ImGui::TableSetupScrollFreeze(cols, rows); }

void TableHeader(const char* label) { ImGui::TableHeader(label); }

void TableHeadersRow() { ImGui::TableHeadersRow(); }
}  // namespace cqsp::client::scripting