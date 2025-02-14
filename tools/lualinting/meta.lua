---@meta

--[[
    What is this file?

    There is a lovely plugin for Visual Studio Code called "Lua" by sumneko.
    It adds lua intillecense and static analasys.

    This file provides the required definitions for that tool

    Don't actually run this file - that would break your environment! 
    But do have it in your VSCode workspace for nice autocomplete features.
    (or have your workspace referencing it - documentation in the plugin.)
]]

-- Referenced from https://github.com/Fesmaster/sol2_ImGui_Bindings/blob/master/meta.lua

ImGui = {}

---push window to the stack and start appending to it.
--- - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
---   which clicking will set the boolean to false when clicked.
--- - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times.
---   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
--- - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
---   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
---   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all other functions
---    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
---    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
--- - Note that the bottom of window stack always contains a window called "Debug".
---@param name string
---@param open boolean?
---@param flags ImGuiWindowFlags?
---@return boolean open, boolean? shouldDraw (open is skipped if open is not provided as a praramater)
function ImGui.Begin(name, open, flags)  end

---pop window from the stack.
function ImGui.End() end

---Push Child Window
--- - Use child windows to begin into a self-contained independent scrolling/clipping
---   regions within a host window. Child windows can embed their own child.
--- - Before 1.90 (November 2023), the "ImGuiChildFlags child_flags = 0" parameter
---   was "bool border = false".
---   
---   This API is backward compatible with old code, 
---   as we guarantee that ImGuiChildFlags_Border == true.
---   Consider updating your old code:
---   ```
---      BeginChild("Name", size, false) -> Begin("Name", size, 0); or 
---                                         Begin("Name", size, ImGuiChildFlags_None);
---      BeginChild("Name", size, true)  -> Begin("Name", size, ImGuiChildFlags_Border);
---   ```
---   
--- - Manual sizing (each axis can use a different setting e.g. ImVec2(0.0f, 400.0f)):
---   - == 0.0f: use remaining parent window size for this axis.
---   - \> 0.0f: use specified size for this axis.
---   -  < 0.0f: right/bottom-align to specified distance from available content boundaries.
---   
--- - Specifying ImGuiChildFlags_AutoResizeX or ImGuiChildFlags_AutoResizeY makes
---   the sizing automatic based on child contents.
---   Combining both ImGuiChildFlags_AutoResizeX _and_ ImGuiChildFlags_AutoResizeY
---   defeats purpose of a scrolling region and is NOT recommended.
---   
--- - BeginChild() returns false to indicate the window is collapsed or fully clipped,
---   so you may early out and omit submitting anything to the window. Always call 
---   a matching EndChild() for each BeginChild() call, regardless of its return value.
---   
---   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are 
---   inconsistent with all other functions such as BeginMenu/EndMenu, BeginPopup/EndPopup,
---   etc. where the EndXXX call should only be called if the corresponding
---   BeginXXX function returned true. Begin and BeginChild are the only odd
---   ones out. Will be fixed in a future update.]
---@param name string
---@param width number?
---@param height number?
---@param childFlags ImGuiChildFlags
---@param windowFlags ImGuiWindowFlags
---@return boolean shouldDraw
function ImGui.BeginChild(name, width, height, childFlags, windowFlags) end

---Pop child window
function ImGui.EndChild() end

---Checks if the current window is appearing
---@return boolean
function ImGui.IsWindowAppearing() end

---Check if the current window is collapsed
---@return boolean
function ImGui.IsWindowCollapsed() end

---is current window focused? or its root/child, 
---depending on flags. see flags for options.
---@param flags ImGuiFocusedFlags
---@return boolean
function ImGui.IsWindowFocused(flags) end

---is current window hovered and hoverable (e.g. not blocked by a popup/modal)? 
---See ImGuiHoveredFlags_ for options. 
---
---IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear ImGui 
---or to your underlying app, you should not use this function! Use the 
---'io.WantCaptureMouse' boolean for that! Refer to FAQ entry "How can I tell whether to 
---dispatch mouse/keyboard to Dear ImGui or my application?" for details.
---@param flags ImGuiHoveredFlags
---@return boolean
function ImGui.IsWindowHovered(flags) end

-- !WARNING! GetWindowDrawList is not supported.

---get DPI scale currently associated to the current window's viewport.
---@return number
function ImGui.GetWindowDpiScale() end

---get current window position in screen space 
---
---(note: it is unlikely you need to use this. Consider using 
---current layout pos instead, GetCursorScreenPos())
---@return number posX, number posY
function ImGui.GetWindowPos() end

---get current window size 
---
---(note: it is unlikely you need to use this. Consider using
--- GetCursorScreenPos() and e.g. GetContentRegionAvail() instead)
---@return number width, number height
function ImGui.GetWindowSize() end

---get current window width (shortcut for GetWindowSize().x)
---@return number width
function ImGui.GetWindowWidth() end

---get current window height (shortcut for GetWindowSize().y)
---@return number height
function ImGui.GetWindowHeight() end

--!WARNING! GetWindowViewport is not supported

---set next window position. call before Begin(). 
---use pivot=(0.5f,0.5f) to center on given point, etc.
---@param posX number
---@param posY number
---@param cond ImGuiCond?
---@param povotX number? Must be supplied with pivotY
---@param pivotY number? Must be supplied with pivotX
function ImGui.SetNextWindowPos(posX, posY, cond, povotX, pivotY ) end

---// set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
---@param width number
---@param height number
---@param cond ImGuiCond?
function ImGui.SetNextWindowSize(width, height, cond) end

---set next window size limits. use 0.0f or FLT_MAX if you don't want limits. 
---Use -1 for both min and max of same axis to preserve current size 
---(which itself is a constraint). 
---Use callback to apply non-trivial programmatic constraints.
---NOTE: callback not currently supported in lua.
---@param minX any
---@param minY any
---@param maxX any
---@param maxY any
function ImGui.SetNextWindowSizeConstraints(minX, minY, maxX, maxY) end

---set next window content size 
---(~ scrollable client area, which enforce the range of scrollbars). 
---Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. 
---set an axis to 0.0f to leave it automatic. call before Begin()
---@param width number
---@param height number
function ImGui.SetNextWindowContentSize(width, height) end

---set next window collapsed state. call before Begin()
---@param collapsed boolean
---@param cond ImGuiCond?
function ImGui.SetNextWindowCollapsed(collapsed, cond) end

---set next window to be focused / top-most. call before Begin()
function ImGui.SetNextWindowFocus() end

--!WARNING! SetNextWindowScroll is not supported

---set next window background color alpha.
--- 
---helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. 
---you may also use ImGuiWindowFlags_NoBackground.
---@param alpha any
function ImGui.SetNextWindowBgAlpha(alpha)end

--!WARNING! SetNextWindowViewport is not supported

---set named window position.
---@param name string
---@param posX number
---@param posY number
---@param cond ImGuiCond?
function ImGui.SetWindowPos(name, posX, posY, cond) end

---set named window size. set axis to 0.0f to force an auto-fit on this axis.
---@param name string
---@param width number
---@param height number
---@param cond ImGuiCond?
function ImGui.SetWindowSize(name, width, height, cond) end

---set named window collapsed state
---@param name string
---@param collapsed boolean
---@param cond ImGuiCond?
function ImGui.SetWindowCollapsed(name, collapsed, cond) end

---set named window to be focused / top-most. use NULL to remove focus.
---@param name string
function ImGui.SetWindowFocus(name) end

---[OBSOLETE] set font scale. 
---
---Adjust IO.FontGlobalScale if you want to scale all windows. 
---This is an old API! For correct scaling, prefer to reload 
---font + rebuild ImFontAtlas + call style.ScaleAllSizes().
---@deprecated Adjust IO.FontGlobalScale if you want to scale all windows.
---@param scale any
function ImGui.SetWindowFontScale(scale) end


---current content boundaries (typically window boundaries including 
---scrolling, or current column boundaries), in windows coordinates
---
---Retrieve available space from a given point. GetContentRegionAvail() 
---is frequently useful.
---Those functions are bound to be redesigned (they are confusing, incomplete
---and the Min/Max return values are in local window coordinates which increases confusion)
---@return number x, number y
function ImGui.GetContentRegionMax() end

---== GetContentRegionMax() - GetCursorPos()
---
---Retrieve available space from a given point. GetContentRegionAvail() 
---is frequently useful.
---Those functions are bound to be redesigned (they are confusing, incomplete
---and the Min/Max return values are in local window coordinates which increases confusion)
---@return number x, number y
function ImGui.GetContentRegionAvail() end

---content boundaries min for the full window (roughly (0,0)-Scroll), in window coordinates
---
---Retrieve available space from a given point. GetContentRegionAvail() 
---is frequently useful.
---Those functions are bound to be redesigned (they are confusing, incomplete
---and the Min/Max return values are in local window coordinates which increases confusion)
---@return number x, number y
function ImGui.GetWindowContentRegionMin() end

---content boundaries max for the full window (roughly (0,0)+Size-Scroll) where Size can 
---be overridden with SetNextWindowContentSize(), in window coordinates
---
---Retrieve available space from a given point. GetContentRegionAvail() 
---is frequently useful.
---Those functions are bound to be redesigned (they are confusing, incomplete
---and the Min/Max return values are in local window coordinates which increases confusion)
---@return number x, number y
function ImGui.GetWindowContentRegionMax() end

---[EMULATED] content boundary width for the full window (roughly (0,0)+Size-Scroll) where Size can
---be overridden with SetNextWindowContentSize(), in window coordinates
---
---Retrieve available space from a given point. GetContentRegionAvail() 
---is frequently useful.
---Those functions are bound to be redesigned (they are confusing, incomplete
---and the Min/Max return values are in local window coordinates which increases confusion)
---@return number width
function ImGui.GetWindowContentRegionWidth() end

function GetDisplaySize() end

---get scrolling amount [0 .. GetScrollMaxX()]
---@return number
function ImGui.GetScrollX() end

---get scrolling amount [0 .. GetScrollMaxY()]
---@return number
function ImGui.GetScrollY() end

---get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
---@return number
function ImGui.GetScrollMaxX() end

---get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
---@return number
function ImGui.GetScrollMaxY() end

---set scrolling amount [0 .. GetScrollMaxX()]
---@param scrollX number
function ImGui.SetScrollX(scrollX)  end

---set scrolling amount [0 .. GetScrollMaxY()]
---@param scrollY number
function ImGui.SetScrollY(scrollY) end

---adjust scrolling amount to make current cursor position visible. 
---
---centerXRatio=
--- - 0.0: left
--- - 0.5: center
--- - 1.0: right
---
---When using to make a "default/current item" visible, 
---consider using SetItemDefaultFocus() instead.
---@param centerXRatio number
function ImGui.SetScrollHereX(centerXRatio) end

---adjust scrolling amount to make current cursor position visible. 
---
---centerYRatio=
--- - 0.0: left
--- - 0.5: center
--- - 1.0: right
---
---When using to make a "default/current item" visible, 
---consider using SetItemDefaultFocus() instead.
---@param centerYRatio number
function ImGui.SetScrollHereY(centerYRatio) end

---adjust scrolling amount to make given position visible.
---
---Generally GetCursorStartPos() + offset to compute a valid position.
---@param localX number
---@param centerXRatio number
function ImGui.SetScrollFromPosX(localX, centerXRatio) end

---adjust scrolling amount to make given position visible.
---
---Generally GetCursorStartPos() + offset to compute a valid position.
---@param localY number
---@param centerYRatio number
function ImGui.SetScrollFromPosY(localY, centerYRatio) end

---Push a font to be used.
---
---use NULL as a shortcut to push default font
---
---font is obtained from your own custom function
---@param font ImFont
function ImGui.PushFont(font) end

---Pop a font from being used
function ImGui.PopFont() end

-- TODO: Add U32 colors

---modify a style color. always use this if you modify the style after NewFrame().
---@param index ImGuiCol
---@param R number [0 .. 1]
---@param G number [0 .. 1]
---@param B number [0 .. 1]
---@param A number [0 .. 1]
function ImGui.PushStyleColor(index, R, G, B, A) end

---Pop modified style color.
---@param count? integer default: 1
function ImGui.PopStyleColor(count) end

---modify a style float/ImVec2 variable. always use this if you modify the style after NewFrame().
---@param index ImGuiStyleVar
---@param valX number
---@param valY number?
function ImGui.PushStyleVar(index, valX, valY) end

---Pop modified style float / ImVec2.
---@param count? integer default: 1
function ImGui.PopStyleVar(count) end

--!WARNING! PushItemFlag is not supported
--!WARNING! PopItemFlag is not supported

---push width of items for common large "item+label" widgets. 
---
--- - \>0.0f: width in pixels, 
--- - <0.0f align xx pixels to the right of window 
---   (so -FLT_MIN always align width to the right side).
---@param itemWidth number
function ImGui.PushItemWidth(itemWidth) end

---Pop width of items
function ImGui.PopItemWidth() end

---set width of the _next_ common large "item+label" widget.
---
--- - \>0.0f: width in pixels, 
--- - <0.0f align xx pixels to the right of window 
---   (so -FLT_MIN always align width to the right side).
---@param itemWidth number
function ImGui.SetNextItemWidth(itemWidth) end

---width of item given pushed settings and current cursor position. 
---NOT necessarily the width of last item unlike most 'Item' functions.
---@return number
function ImGui.CalcItemWidth() end

---push word-wrapping position for Text*() commands. 
---
--- - < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); 
--- - \> 0.0f: wrap at 'wrap_pos_x' position in window local space
---@param wrapLocalPosX number?
function ImGui.PushTextWrapPos(wrapLocalPosX) end

---Pop word-wrapping position for Text*() commands.
function ImGui.PopTextWrapPos() end


---get current font
---@return ImFont font
function ImGui.GetFont() end

---get current font size (= height in pixels) of current font with current scale applied
---@return number
function ImGui.GetFontSize() end

---get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API
---@return number x, number y
function  ImGui.GetFontTexUvWhitePixel() end

-- TODO: Add U32 colors

---retrieve style color as stored in ImGuiStyle structure. 
---
---use to feed back into PushStyleColor(), otherwise use GetColorU32() 
---to get style color with style alpha baked in.
---@param idx ImGuiCol
---@return number r, number g, number b, number a
function ImGui.GetStyleColorVec4(idx) end


---cursor position in absolute coordinates (prefer using this, also more useful to work with ImDrawList API).
---@return number x, number y
function  ImGui.GetCursorScreenPos() end

---cursor position in absolute coordinates
---@param x number
---@param y number
function ImGui.SetCursorScreenPos(x, y) end

---[window-local] get cursor position in window coordinates (relative to window position)
---@return number x, number y
function  ImGui.GetCursorPos() end

---[window-local] get cursor position in window coordinates (relative to window position)
---@return number x
function  ImGui.GetCursorPosX() end

---[window-local] get cursor position in window coordinates (relative to window position)
---@return number y
function  ImGui.GetCursorPosY() end

---[window-local] set cursor position in window coordinates (relative to window position)
---@param x number
---@param y number
function ImGui.SetCursorPos(x, y) end

---[window-local] set cursor position in window coordinates (relative to window position)
---@param x number
function ImGui.SetCursorPosX(x) end

---[window-local] set cursor position in window coordinates (relative to window position)
---@param y number
function ImGui.SetCursorPosY(y) end

---[window-local] initial cursor position, in window coordinates
---@return number x, number y
function ImGui.GetCursorStartPos() end

---separator, generally horizontal. 
---
---inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
function ImGui.Separator()  end

---call between widgets or groups to layout them horizontally. 
---
---X position given in window coordinates.
---@param offset_from_start_x number
---@param spacing number
function ImGui.SameLine(offset_from_start_x, spacing) end

---undo a SameLine() or force a new line when in a horizontal-layout context.
function ImGui.NewLine() end

---add vertical spacing.
function ImGui.Spacing() end

---add a dummy item of given size. 
---
---unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
---@param sizeX number
---@param sizeY number
function ImGui.Dummy(sizeX, sizeY) end

---move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
---@param indent_w number
function ImGui.Indent(indent_w) end

---move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
---@param indent_w number
function ImGui.Unindent(indent_w) end

---lock horizontal starting position
function ImGui.BeginGroup() end

---unlock horizontal starting position + capture the whole group bounding box into 
---one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() 
---on whole group, etc.)
function ImGui.EndGroup() end

---vertically align upcoming text baseline to FramePadding.y so that it will align 
---properly to regularly framed items (call if you have text on a line before a framed item)
function ImGui.AlignTextToFramePadding() end

---~ FontSize
---@return number
function ImGui.GetTextLineHeight() end

---~ FontSize + style.ItemSpacing.y 
---(distance in pixels between 2 consecutive lines of text)
---@return number
function ImGui.GetTextLineHeightWithSpacing() end

---~ FontSize + style.FramePadding.y * 2
---@return number
function ImGui.GetFrameHeight() end

---~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y 
---(distance in pixels between 2 consecutive lines of framed widgets)
---@return number
function ImGui.GetFrameHeightWithSpacing() end

---push an ID onto the stack
---@param id_begin string|integer
---@param id_end string?
function ImGui.PushID(id_begin, id_end) end

---Pop an ID from the stack
function ImGui.PopID() end

---calculate unique ID (hash of whole ID stack + given parameter). 
---e.g. if you want to query into ImGuiStorage yourself
---@param id_begin string|integer
---@param id_end string?
---@return ImGuiID id
function ImGui.GetID(id_begin, id_end) end

---raw text without formatting.
---
---Roughly equivalent to Text("%s", text) but: 
--- 1. doesn't require null terminated string if 'text_end' is specified, 
--- 2. it's faster, no memory copy is done, no buffer size limits, 
---    recommended for long chunks of text.
---@param text string
---@param text_end string?
function ImGui.TextUnformatted(text, text_end) end

---formatted text
---@param fmt string
function ImGui.Text(fmt) end

---Colored text
---
---shortcut for 
---```
---PushStyleColor(ImGuiCol_Text, col); 
---Text(fmt, ...); 
---PopStyleColor();
---```
---@param r number
---@param g number
---@param b number
---@param a number
---@param fmt string
function ImGui.TextColored(r, g, b, a, fmt) end

---Disabled text
---
---shortcut for: 
---```
---PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); 
---Text(fmt, ...); 
---PopStyleColor();
---```
---@param fmt string
function ImGui.TextDisabled(fmt) end

---Wrapped test
---
---shortcut for 
---```
---PushTextWrapPos(0.0f); 
---Text(fmt, ...); 
---PopTextWrapPos(); 
---```
---
--Note that this won't work on an auto-resizing window if there's no other widgets to 
---extend the window width, you may need to set a size using SetNextWindowSize().
---@param fmt string
function ImGui.TextWrapped(fmt) end

---display text+label aligned the same way as value+label widgets
---@param label string
---@param fmt string
function ImGui.LabelText(label, fmt) end

---Text with a bullet point
---
---shortcut for Bullet()+Text()
---@param fmt string
function ImGui.BulletText(fmt) end

---button
---@param label string
---@param sizeX number
---@param sizeY number
---@return boolean pressed
function  ImGui.Button(label, sizeX, sizeY) end

---button with (FramePadding.y == 0) to easily embed within text
---@param label string
---@return boolean pressed
function ImGui.SmallButton(label) end

---flexible button behavior without the visuals,
---frequently useful to build custom behaviors using the public api 
---(along with IsItemActive, IsItemHovered, etc.)
---@param label string
---@param sizeX number
---@param sizeY number
---@return boolean pressed
function ImGui.InvisibleButton(label, sizeX, sizeY) end

---quare button with an arrow shape
---@param label string
---@param dir ImGuiDir
---@return boolean pressed
function ImGui.ArrowButton(label, dir) end

---Checkbox
---@param label string
---@param value boolean
---@return boolean value, boolean pressed
function ImGui.Checkbox(label, value) end

--!WARNING! CheckboxFlags is not supported

---Radiobutton - given N buttons, each with a unique instance_id, only one can be selected.
---@param label string
---@param selected_id integer
---@param instance_id integer
---@return boolean pressed
function ImGui.RadioButton(label, selected_id, instance_id) end

---Progress bar
---@param fraction number
---@param width number? must be used with height
---@param height number? must be used with width
---@param overlay string?
function ImGui.ProgressBar(fraction, width, height, overlay) end

---draw a small circle + keep the cursor on the same line.
---advance cursor x position by GetTreeNodeToLabelSpacing(), 
---same distance that TreeNode() uses
function ImGui.Bullet() end

--!WARNING! TextLink is not supported
--!WARNING! TextLinkOpenURL is not supported

--!WARNING! Image is not supported
--!WARNING! ImageButton is not supported

---Start a combo box (Dropdown)
---@param label string
---@param preview string
---@param flags ImGuiComboFlags?
---@return boolean should_draw
function ImGui.BeginCombo(label, preview, flags) end

---End a combo box (Dropdown)
---WARNING: Only call if BeginCombo() returned true
function ImGui.EndCombo() end

---Combo box entry
---@param label string
---@param current_item integer
---@param items string[] | string if just a string, use "\0" to seperate entries AND at the end of the string.
---@param item_count integer must match items
---@param popup_max_height_in_items integer?
---@return integer selection, boolean clicked
function ImGui.Combo(label, current_item, items, item_count, popup_max_height_in_items) end

---Dragable float values. Ctrl+Click will allow text input
---@param label string
---@param value number
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number value, boolean clicked
function ImGui.DragFloat(label, value, speed, min, max, format, flags) end

---Dragable float values. Ctrl+Click will allow text input
---@param label string
---@param value number[] len=2
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number[] value, boolean clicked
function ImGui.DragFloat2(label, value, speed, min, max, format, flags) end

---Dragable float values. Ctrl+Click will allow text input
---@param label string
---@param value number[] len=3
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number[] value, boolean clicked
function ImGui.DragFloat3(label, value, speed, min, max, format, flags) end

---Dragable float values. Ctrl+Click will allow text input
---@param label string
---@param value number[] len=4
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number[] value, boolean clicked
function ImGui.DragFloat4(label, value, speed, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer value, boolean clicked
function ImGui.DragInt(label, value, speed, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer[] len=2
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer[] value, boolean clicked
function ImGui.DragInt2(label, value, speed, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer[] len=3
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer[] value, boolean clicked
function ImGui.DragInt3(label, value, speed, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer[] len=4
---@param speed number? the inverse number of pixels the mouse needs to move to adjust the value by 1.0
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer[] value, boolean clicked
function ImGui.DragInt4(label, value, speed, min, max, format, flags) end

--!WARNING! DragIntRange2 is not supported
--!WARNING! DragScalar is not supported
--!WARNING! DragScalarN is not supported

---Regular slider float values. Ctrl+Click will allow text input
---@param label string
---@param value number
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number value, boolean clicked
function ImGui.SliderFloat(label, value, min, max, format, flags) end

---Regular slider float values. Ctrl+Click will allow text input
---@param label string
---@param value number[] len=2
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number[] value, boolean clicked
function ImGui.SliderFloat2(label, value, min, max, format, flags) end

---Regular slider float values. Ctrl+Click will allow text input
---@param label string
---@param value number[] len=3
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number[] value, boolean clicked
function ImGui.SliderFloat3(label, value, min, max, format, flags) end

---Regular slider float values. Ctrl+Click will allow text input
---@param label string
---@param value number[] len=4
---@param min number? min value of the draggable range. Manual input can exceed this.
---@param max number? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return number[] value, boolean clicked
function ImGui.SliderFloat4(label, value, min, max, format, flags) end

---Angle slider - allows the underlying value to be radians but the display to be degrees.
---@param label string
---@param v_rad number
---@param v_degrees_min number?
---@param v_degrees_max number?
---@param format string?
---@param flags ImGuiSliderFlags?
function ImGui.SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer value, boolean clicked
function ImGui.SliderInt(label, value, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer[] len=2
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer[] value, boolean clicked
function ImGui.SliderInt2(label, value, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer[] len=3
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer[] value, boolean clicked
function ImGui.SliderInt3(label, value, min, max, format, flags) end

---Dragable integer values. Ctrl+Click will allow text input
---@param label string
---@param value integer[] len=4
---@param min integer? min value of the draggable range. Manual input can exceed this.
---@param max integer? max value of the draggable range. Manual input can exceed this
---@param format string? Text for formatting display. Uses printf formattting.
---@param flags ImGuiSliderFlags?
---@return integer[] value, boolean clicked
function ImGui.SliderInt4(label, value, min, max, format, flags) end

--!WARNING! SliderScalar is nott implemented
--!WARNING! SliderScalarN is nott implemented

---VSliderFloat
---@param label string
---@param width number
---@param height number
---@param value number
---@param min number
---@param max number
---@param format string?
---@param flags ImGuiSliderFlags?
---@return number value, boolean clicked
function ImGui.VSliderFloat(label, width, height, value, min, max, format, flags) end

---VSliderInt
---@param label string
---@param width number
---@param height number
---@param value integer
---@param min integer
---@param max integer
---@param format string?
---@param flags ImGuiSliderFlags?
---@return integer value, boolean clicked
function ImGui.VSliderInt(label, width, height, value, min, max, format, flags) end

--!WARNING! VSliderScalar is nott implemented

---Text Input with Keyboard
---@param label string
---@param text string
---@param flags ImGuiInputTextFlags?
---@return string text, boolean clicked
function ImGui.InputText(label, text, flags) end

---Multiline text input with Keyboard
---@param label string
---@param text string
---@param sizeX number?
---@param sizeY number?
---@param flags ImGuiInputTextFlags?
---@return string text, boolean clicked
function ImGui.InputTextMultiline(label, text, sizeX, sizeY, flags) end

---text input with Keyboard and a hint
---@param label string
---@param hint string
---@param text string
---@param flags ImGuiInputTextFlags?
---@return string text, boolean clicked
function ImGui.InputTextWithHint(label, hint, text, flags) end

---text input for a floating-point number
---@param label string
---@param value number
---@param step number?
---@param step_fast number?
---@param format string?
---@param flags ImGuiInputTextFlags?
---@return number value, boolean clicked
function ImGui.InputFloat(label, value, step, step_fast, format, flags) end

---text input for a floating-point number
---@param label string
---@param values number[] len=2
---@param step number?
---@param step_fast number?
---@param format string?
---@param flags ImGuiInputTextFlags?
---@return number[] values, boolean clicked
function ImGui.InputFloat2(label, values, step, step_fast, format, flags) end

---text input for a floating-point number
---@param label string
---@param values number[] len=3
---@param step number?
---@param step_fast number?
---@param format string?
---@param flags ImGuiInputTextFlags?
---@return number[] values, boolean clicked
function ImGui.InputFloat3(label, values, step, step_fast, format, flags) end

---text input for a floating-point number
---@param label string
---@param values number[] len=4
---@param step number?
---@param step_fast number?
---@param format string?
---@param flags ImGuiInputTextFlags?
---@return number[] values, boolean clicked
function ImGui.InputFloat4(label, values, step, step_fast, format, flags) end

---text input for a integer
---@param label string
---@param value integer
---@param step integer?
---@param step_fast integer?
---@param flags ImGuiInputTextFlags?
---@return integer value, boolean clicked
function ImGui.InputInt(label, value, step, step_fast, flags) end

---text input for a integer
---@param label string
---@param values integer[] len=2
---@param flags ImGuiInputTextFlags?
---@return integer[] values, boolean clicked
function ImGui.InputInt2(label, values, flags) end

---text input for a integer
---@param label string
---@param values integer[] len=3
---@param flags ImGuiInputTextFlags?
---@return integer[] values, boolean clicked
function ImGui.InputInt3(label, values, flags) end

---text input for a integer
---@param label string
---@param values integer[] len=4
---@param flags ImGuiInputTextFlags?
---@return integer[] values, boolean clicked
function ImGui.InputInt4(label, values, flags) end

---text input for a dobule-precision floating-point number
---@param label string
---@param value number
---@param step number?
---@param step_fast number?
---@param format string?
---@param flags ImGuiInputTextFlags?
---@return number value, boolean clicked
function ImGui.InputDouble(label, value, step, step_fast, format, flags) end

---Color picker
---@param label string
---@param col number[] len=3
---@param flags ImGuiColorEditFlags?
---@return number[] color, boolean clicked
function ImGui.ColorEdit3(label, col, flags) end

---Color picker
---@param label string
---@param col number[] len=4
---@param flags ImGuiColorEditFlags?
---@return number[] color, boolean clicked
function ImGui.ColorEdit4(label, col, flags) end

---Color picker
---@param label string
---@param col number[] len=3
---@param flags ImGuiColorEditFlags?
---@return number[] color, boolean clicked
function ImGui.ColorPicker3(label, col, flags) end

---Color picker
---@param label string
---@param col number[] len=4
---@param flags ImGuiColorEditFlags?
---@return number[] color, boolean clicked
function ImGui.ColorPicker4(label, col, flags) end

---display a color square/button, hover for details, return true when pressed.
---@param label string
---@param col number[] len=4
---@param flags ImGuiColorEditFlags?
---@param width number?
---@param height number?
---@return boolean clicked
function ImGui.ColorButton(label, col, flags, width, height) end

---initialize current options (generally on application startup) 
---if you want to select a default format, picker type, etc. 
---User will be able to change many settings, unless you pass the _NoOptions flag to your calls.
---@param flags ImGuiColorEditFlags
function ImGui.SetColorEditOptions(flags) end

---A node in a collapsable tree. Returns true if the tree is open, 
---in which case, you need to call TreePop() to finish displaying the contents.
---@param label string
---@param fmt string
---@return boolean is_open
function ImGui.TreeNode(label, fmt) end

---A node in a collapsable tree. Returns true if the tree is open, 
---in which case, you need to call TreePop() to finish displaying the contents.
---@param label string
---@param flags ImGuiTreeNodeFlags
---@param fmt string
---@return boolean is_open
function ImGui.TreeNodeEx(label, flags, fmt) end

---~ Indent()+PushID(). 
---Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
---@param str_id any
function ImGui.TreePush(str_id) end

---~ Unindent()+PopID()
function ImGui.TreePop() end

---horizontal distance preceding label when using 
---TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
---@return number
function ImGui.GetTreeNodeToLabelSpacing() end

---Create a header
---if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
---@param label string
---@param open boolean|ImGuiTreeNodeFlags|nil If open is skipped, takes the place of flags
---@param flags ImGuiTreeNodeFlags?
---@return boolean visible_or_open, boolean? visible when open param is not provided, open return is ignored.
function ImGui.CollapsingHeader(label, open, flags) end

---set next TreeNode/CollapsingHeader open state.
---@param is_open boolean
---@param condition ImGuiCond?
function ImGui.SetNextItemOpen(is_open, condition) end

---A selectable highlights when hovered, and can display another color when selected.
---Neighbors selectable extend their highlight bounds in order to leave no gap between them. 
---This is so a series of selected Selectable appear contiguous.
---@param label string
---@param selected boolean?
---@param flags ImGuiSelectableFlags?
---@param width number?
---@param height number?
---@return boolean selected
function ImGui.Selectable(label, selected, flags, width, height) end

--!WARNING! BeginMultiSelect is not implemented
--!WARNING! EndMultiSelect is not implemented
--!WARNING! SetNextItemSelectionUserData is not implemented
--!WARNING! IsItemToggledSelection is not implemented

---Begin a List Box
---@param label string
---@param width number?
---@param height number?
---@return boolean visible
function ImGui.BeginListBox(label, width, height) end

---End a list box
function ImGui.EndListBox() end

---ListBox
---This is essentially a thin wrapper to using BeginChild/EndChild with the ImGuiChildFlags_FrameStyle flag 
---for stylistic changes + displaying a label.
---
---You can submit contents and manage your selection state however you want it, by creating e.g. 
---Selectable() or any other items.
---
---The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept 
---available for convenience purpose. This is analoguous to how Combos are created.
---
---Choose frame width:   
--- - size.x > 0.0f: custom
--- - size.x < 0.0f or -FLT_MIN: right-align  
--- - size.x = 0.0f (default): use current ItemWidth
---
---Choose frame height:  
--- - size.y > 0.0f: custom
--- - size.y < 0.0f or -FLT_MIN: bottom-align
--- - size.y = 0.0f (default): arbitrary default height which can fit ~7 items
---@param label string
---@param current_item integer
---@param items string[]
---@param item_count integer #items
---@param height_in_items integer?
---@return integer active_index, boolean clicked
function ImGui.ListBox(label, current_item, items, item_count, height_in_items) end

--!WARNING! PlotLines is not impllemented
--!WARNING! PlotHistogram is not impllemented

---Value() helper
---Those are merely shortcut to calling Text() with a format string. 
---Output single value in "name: value" format 
---(tip: freely declare more in your code to handle your types. 
---you can add functions to the ImGui namespace)
---@param Prefix string
---@param value boolean|integer|number
---@param float_format string used to format numbers when a floating point number is used.
function ImGui.Value(Prefix, value, float_format) end

---Use BeginMenuBar() on a window ImGuiWindowFlags_MenuBar to append to its menu bar.
function ImGui.BeginMenuBar() end

---End a MenuBar
function ImGui.EndMenuBar() end

---Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
function ImGui.BeginMainMenuBar() end

---End main menu bar on top of screen
function ImGui.EndMainMenuBar() end

---create a sub-menu entry. only call EndMenu() if this returns true!
---@param label string
---@param enabled boolean?
---@return boolean enabled
function ImGui.BeginMenu(label, enabled) end

---End a sub-menu entry.
function ImGui.EndMenu() end

---Create an entry in a sub-menu
---return true when activated + toggle (*selected) if selected != NULL
---@param label string
---@param shortcut string?
---@param selected boolean? if supplied, returns two values
---@param enabled boolean?
---@return boolean selected_or_clicked, boolean? clicked
function ImGui.MenuItem(label, shortcut, selected, enabled) end

---Create a tooltip
---
---Tooltips are windows following the mouse. They do not take focus away.
---A tooltip window can contain items of any types.
---only call EndTooltip() if BeginTooltip()/BeginItemTooltip() returns true!
---@return boolean open
function ImGui.BeginTooltip() end

---End a tooltip that is open
function ImGui.EndTooltip() end

---set a text-only tooltip. Often used after a ImGui::IsItemHovered() check. 
---Override any previous call to SetTooltip().
---@param fmt string
function ImGui.SetTooltip(fmt) end


---Create a popup
---return true if the popup is open, and you can start outputting to it.
---@param str_id string
---@param flags ImGuiWindowFlags?
---@return boolean open
function ImGui.BeginPopup(str_id, flags) end

---Create a popup modal (no interaction with anything but the modal)
---return true if the modal is open, and you can start outputting to it.
---@param name string
---@param open boolean?
---@param flags ImGuiWindowFlags?
---@return boolean open
function ImGui.BeginPopupModal(name, open, flags) end

---End an open popup
---only call EndPopup() if BeginPopupXXX() returns true!
function ImGui.EndPopup() end

---OpenPopup(): set popup state to open. ImGuiPopupFlags are available for opening options.
---@param str_id string
---@param flags ImGuiPopupFlags?
function ImGui.OpenPopup(str_id, flags) end

--!WARNING! OpenPopupOnItemClick is not implemented

---CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
function ImGui.CloseCurrentPopup() end

---open+begin popup when clicked on last item. 
---Use str_id==NULL to associate the popup to previous item. 
---If you want to use that on a non-interactive item such as Text() you 
---need to pass in an explicit ID here. read comments in .cpp!
---@param str_id string?
---@param flags ImGuiPopupFlags?
---@return boolean open
function ImGui.BeginPopupContextItem(str_id, flags) end

---open+begin popup when clicked on current window.
---@param str_id string?
---@param flags ImGuiPopupFlags?
---@return boolean open
function ImGui.BeginPopupContextWindow(str_id, flags) end

---open+begin popup when clicked in void (where there are no windows).
---@param str_id string?
---@param flags ImGuiPopupFlags?
---@return boolean open
function ImGui.BeginPopupContextVoid(str_id, flags) end

---Popups: query functions
--- - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
--- - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
--- - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
---@param str_id string
---@param flags ImGuiPopupFlags?
---@return boolean
function ImGui.IsPopupOpen(str_id, flags) end

--!WARNING! Table API not implemented:
--  BeginTable
--  EndTable
--  TableNextRow
--  TableNextColumn
--  TableSetColumnIndex
--  TableSetupColumn
--  TableSetupScrollFreeze
--  TableHeader
--  TableHeadersRow
--  TableAngledHeadersRow
--  TableGetSortSpecs
--  TableGetColumnCount
--  TableGetColumnIndex
--  TableGetRowIndex
--  TableGetColumnName
--  TableGetColumnFlags
--  TableSetColumnEnabled
--  TableGetHoveredColumn
--  TableSetBgColor

---Start doing columns.
---@param count integer?
---@param id string?
---@param border boolean?
---@deprecated prefer using Tables!
function ImGui.Columns(count, id, border) end

---next column, defaults to current row or next row if the current row is finished
---@deprecated prefer using Tables!
function ImGui.NextColumn() end

---get current column index
---@return integer
---@deprecated prefer using Tables!
function ImGui.GetColumnIndex() end

---get column width (in pixels). pass -1 to use current column
---@param column_index integer?
---@return number
---@deprecated prefer using Tables!
function ImGui.getColumnWidth(column_index) end

---set column width (in pixels). pass -1 to use current column
---@param column_index integer
---@param width number
---@deprecated prefer using Tables!
function ImGui.SetColumnWidth(column_index, width) end

---get position of column line (in pixels, from the left side of the contents region). 
---pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. 
---column 0 is typically 0.0f
---@param column_index integer?
---@return number
---@deprecated prefer using Tables!
function ImGui.GetColumnOffset(column_index) end

---set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
---@param column_index integer
---@param offset_x number
---@deprecated prefer using Tables!
function ImGui.SetColumnOffset(column_index, offset_x) end

---Get number of columns
---@return integer
---@deprecated prefer using Tables!
function ImGui.GetColumnsCount() end


---Manual tabs not created by docking system
---@param str_id string
---@param flags ImGuiTabBarFlags
---@return boolean open
function ImGui.BeginTabBar(str_id, flags) end

---End tab bar
---only call EndTabBar() if BeginTabBar() returns true!
function ImGui.EndTabBar() end

---create a Tab. Returns true if the Tab is selected.
---@param label string
---@param open boolean?
---@param flags ImGuiTabItemFlags?
---@return boolean open_or_selected, boolean selected
function ImGui.BeginTabItem(label, open, flags) end

---end a tab
---only call EndTabItem() if BeginTabItem() returns true!
function ImGui.EndTabItem() end

---Notify TabBar or Docking system of a closed tab/window ahead 
---(useful to reduce visual flicker on reorderable tab bars). 
---For tab-bar: call after BeginTabBar() and before Tab submissions. 
---Otherwise call with a window name.
---@param tab_or_docked_window_label string
function ImGui.SetTabItemClosed(tab_or_docked_window_label) end


---A dockspace inside of a windows
---@param dockspace_id integer
---@param width number?
---@param height number?
---@param flags ImGuiDockNodeFlags?
function ImGui.DockSpace(dockspace_id, width, height, flags) end

--!WARNING! DockSpaceOverViewport is not implemented

---set next window dock id
---@param dockspace_id integer
---@param condition integer?
function ImGui.SetNextWindowDockID(dockspace_id, condition) end

--!WARNING! SetNextWindowClass is not implemented

---Get the ID for the dockspace window is docked in
---@return integer
function ImGui.GetWindowDockID() end

---is current window docked into another window?
---@return boolean
function ImGui.IsWindowDocked() end


---start logging to tty (stdout)
---@param auto_open_depth integer
function ImGui.LogToTTY(auto_open_depth) end

---start logging to file
---@param auto_open_depth integer
---@param filename string
function ImGui.LogToFile(auto_open_depth, filename) end

---start logging to OS clipboard
---@param auto_open_depth integer
function ImGui.LogToClipboard(auto_open_depth) end

---stop logging (close file, etc.)
function ImGui.LogFinish() end

---helper to display buttons for logging to tty/file/clipboard
function ImGui.LogButtons() end

---pass text data straight to log (without being displayed)
---@param fmt string
function ImGui.LogText(fmt) end

--!WARNING! Drag and Drop api is not implemented

--!WARNING! Disabling api is not implemented

--[[ Clipping ]]

---Clip visuals and mouse hovering
---@param minX number
---@param minY number
---@param maxX number
---@param maxY number
---@param intersect_with_current_clip_rect boolean
function ImGui.PushClipRect(minX, minY, maxX, maxY, intersect_with_current_clip_rect) end

---Pop a pushed clip rect
function ImGui.PopClipRect() end

--[[ Focus, Activation ]]

---make last item the default focused item of a window.
function ImGui.SetItemDefaultFocus() end

---focus keyboard on the next widget. 
---Use positive 'offset' to access sub components of a multiple component widget. 
---Use -1 to access previous widget.
---@param offset integer
function ImGui.SetKeyboardFocusHere(offset) end


--[[ Overlapping mode ]]

---allow next item to be overlapped by a subsequent item. 
---Useful with invisible buttons, selectable, treenode covering an area 
---where subsequent items may need to be added. 
---Note that both Selectable() and TreeNode() have dedicated flags doing this.
function ImGui.SetNextItemAllowOverlap() end

--[[ Item/Widgets Utilities and Query Functions ]]

---is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.
---@param flags ImGuiHoveredFlags?
---@return boolean
function ImGui.IsItemHovered(flags) end

---is the last item active? (e.g. button being held, text field being edited. 
---This will continuously return true while holding mouse button on an item. 
---Items that don't interact will always return false)
---@return boolean
function ImGui.IsItemActive() end

---is the last item focused for keyboard/gamepad navigation?
---@return boolean
function ImGui.IsItemFocused() end

---is the last item hovered and mouse clicked on? 
--(**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) 
---this is NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
---@param mouse_button ImGuiMouseButton?
---@return boolean
function ImGui.IsItemClicked(mouse_button) end

---is the last item visible? (items may be out of sight because of clipping/scrolling)
---@return boolean
function ImGui.IsItemVisible() end

---did the last item modify its underlying value this frame? or was pressed? 
---This is generally the same as the "bool" return value of many widgets.
---@return boolean
function ImGui.IsItemEdited() end

---was the last item just made active (item was previously inactive).
---@return boolean
function ImGui.IsItemActivated() end

---was the last item just made inactive (item was previously active).
---Useful for Undo/Redo patterns with widgets that require continuous editing.
---@return boolean
function ImGui.IsItemDeactivated() end

---was the last item just made inactive and made a value change when it was active? 
---(e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that require continuous editing. 
---Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true 
---even when clicking an already selected item).
---@return boolean
function ImGui.IsItemDeactivatedAfterEdit() end

---was the last item open state toggled? set by TreeNode().
---@return boolean
function ImGui.IsItemToggledOpen() end

---is any item hovered?
---@return boolean
function ImGui.IsAnyItemHovered() end

---is any item active?
---@return boolean
function ImGui.IsAnyItemActive() end

---is any item focused?
---@return boolean
function ImGui.IsAnyItemFocused() end

--!WARNING! GetItemID is not implemented

---get upper-left bounding rectangle of the last item (screen space)
---@return number x, number y
function ImGui.GetItemRectMin() end

---get lower-right bounding rectangle of the last item (screen space)
---@return number x, number y
function ImGui.GetItemRectMax() end

---get size of last item
---@return number width, number height
function ImGui.GetItemRectSize() end

--!WARNING! Viewports API is not implemented

--!WARNING! Background/Foreground Draw Lists API is not implemented

--[[ Miscellaneous Utilities ]]

---test if rectangle (of given size, starting from cursor position) is visible / not clipped.
---@param size_minX number, sizex if two params used, minX if four params used
---@param size_minY number, sizeY if two params used, minY if four params used
---@param maxX number?
---@param maxY number?
---@return boolean
function ImGui.IsRectVisible(size_minX, size_minY, maxX, maxY) end

---get global imgui time. incremented by io.DeltaTime every frame.
---@return number
function ImGui.GetTime() end

---get global imgui frame count. incremented by 1 every frame.
---@return integer
function ImGui.GetFrameCount() end

--!WARNING! GetDrawListSharedData is not implemented

---get a string corresponding to the enum value (for display, saving, etc.).
---@param index ImGuiCol
---@return string
function ImGui.GetStyleColorName(index) end

--!WARNING! SetStateStorage is not implemented
--!WARNING! GetStateStorage is not implemented

--[[ Text Utilities ]]

---Caclulate the size of a text block
---@param text string
---@param ending string?
---@param hide_text_after_double_hash boolean?
---@param wrap_width number?
---@return number width, number height
function ImGui.CalcTextSize(text, ending, hide_text_after_double_hash, wrap_width) end

--[[-----------------------------------------------------------------------------------------------
    Color Utilities 
-------------------------------------------------------------------------------------------------]]

--TODO: Add U32 Utilities documentation

---Convert RGB color to HSV
---@param r number
---@param g number
---@param b number
---@return number h, number s, number v
function ImGui.ColorConvertRGBtoHSV(r, g, b) end

---Convert HSV color to RGB
---@param h number
---@param s number
---@param v number
---@return number r, number g, number b
function ImGui.ColorConvertHSVtoRGB(h, s, v) end

--[[-----------------------------------------------------------------------------------------------
    Inputs Utilities: Keyboard/Mouse/Gamepad 
-------------------------------------------------------------------------------------------------]]

---is key being held.
---@param key ImGuiKey
---@return boolean
function ImGui.IsKeyDown(key) end

---was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
---@param key ImGuiKey
---@param repeat_ boolean?
---@return boolean
function ImGui.IsKeyPressed(key, repeat_) end

---was key released (went from Down to !Down)?
---@param key ImGuiKey
---@return boolean
function ImGui.IsKeyReleased(key) end

--!WARNING! IsKeyChordPressed is not implemented

function ImGui.GetKeyPressedAmount(key, repeat_delay, rate) end

--!WARNING! GetKeyName is not implemented

function ImGui.SetNextFrameWantCaptureKeyboard(want_capture_keyboard) end

--!WARNING! Inputs Utilities: Shortcut Testing & Routing [BETA] API is not implemented

--!WARNING! Inputs Utilities: Key/Input Ownership [BETA] API is not implemented

--[[-----------------------------------------------------------------------------------------------
    Inputs Utilities: Mouse specific
-------------------------------------------------------------------------------------------------]]

---is mouse button held?
---@param button ImGuiMouseButton
---@return boolean
function ImGui.IsMouseDown(button) end

---did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
---@param button ImGuiMouseButton
---@param repeat_ boolean?
---@return boolean
function ImGui.IsMouseClicked(button, repeat_) end

---did mouse button released? (went from Down to !Down)
---@param button ImGuiMouseButton
---@return boolean
function ImGui.IsMouseReleased(button) end

---did mouse button double-clicked? 
---Same as GetMouseClickedCount() == 2. 
---(note that a double-click will also report IsMouseClicked() == true)
---@param button ImGuiMouseButton
---@return boolean
function ImGui.IsMouseDoubleClicked(button) end

---is mouse hovering given bounding rect (in screen space). 
---clipped by current clipping settings, but disregarding of other consideration of 
---focus/window ordering/popup-block.
---@param minX number
---@param minY number
---@param maxX number
---@param maxY number
---@param clip boolean?
---@return boolean
function ImGui.IsMouseHoveringRect(minX, minY, maxX, maxY, clip) end

--!WARNING! IsMousePosValid is not implemented

---[WILL OBSOLETE] is any mouse button held? This was designed for backends, 
---but prefer having backend maintain a mask of held mouse buttons, 
---because upcoming input queue system will make this invalid.
---@return boolean
---@deprecated soon to be obselete
function ImGui.IsAnyMouseDown() end

---shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
---@return number x , number y
function ImGui.GetMousePos() end

---retrieve mouse position at the time of opening popup we have BeginPopup() into 
---(helper to avoid user backing that value themselves)
---@return number x , number y
function ImGui.GetMousePosOnOpeningCurrentPopup() end

---is mouse dragging? (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
---@param button ImGuiMouseButton
---@param lock_threshold number?
---@return boolean
function ImGui.IsMouseDragging(button, lock_threshold) end

---return the delta from the initial clicking position while the mouse button 
---is pressed or was just released. This is locked and return 0.0f until the 
---mouse moves past a distance threshold at least once 
---(uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
---@param button ImGuiMouseButton?
---@param lock_threshold number?
---@return number dx, number dy
function ImGui.GetMouseDragDelta(button, lock_threshold) end

---Reset the mouse drag delta
---@param button ImGuiMouseButton?
function ImGui.ResetMouseDragDelta(button) end

---get desired mouse cursor shape. 
---
---Important: reset in ImGui::NewFrame(), this is updated during the frame. 
---valid before Render(). 
---If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
---@return ImGuiMouseCursor
function ImGui.GetMouseCursor() end

---set desired mouse cursor shape
---@param cursor_type ImGuiMouseCursor
function ImGui.SetMouseCursor(cursor_type) end

---Override io.WantCaptureMouse flag next frame 
---(said flag is left for your application to handle, typical when true it instucts your app to ignore inputs). 
---This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.
---@param want_capture_mouse boolean
function ImGui.SetNextFrameWantCaptureMouse(want_capture_mouse) end

--[[-----------------------------------------------------------------------------------------------
    Clipboard Utilities
-------------------------------------------------------------------------------------------------]]

---Get text in the OS clipboard
---@return string
function ImGui.GetClipboardText() end

---Set text in the OS clipboard
---@param text string
function ImGui.SetClipboardText(text) end

--[[-----------------------------------------------------------------------------------------------
    Enum Values

    NOTE: In this meta file, all enum values have the value 0. This is simply for the santiy of
    maintaining this file, and does not affect the real values of these enums
-------------------------------------------------------------------------------------------------]]
---@alias ImFont userdata
---@alias ImGuiID integer


---@enum ImGuiWindowFlags
ImGuiWindowFlags = {
    None = 0,
    NoTitleBar = 0,
    NoResize = 0,
    NoMove = 0,
    NoScrollbar = 0,
    NoScrollWithMouse = 0,
    NoCollapse = 0,
    AlwaysAutoResize = 0,
    NoBackground = 0,
    NoSavedSettings = 0,
    NoMouseInputs = 0,
    MenuBar = 0,
    HorizontalScrollbar = 0,
    NoFocusOnAppearing = 0,
    NoBringToFrontOnFocus = 0,
    AlwaysVerticalScrollbar = 0,
    AlwaysHorizontalScrollbar = 0,
    NoNavInputs = 0,
    NoNavFocus = 0,
    UnsavedDocument = 0,
    NoDocking = 0,
    NoNav = 0,
    NoDecoration = 0,
    NoInputs = 0,
    ChildWindow = 0,
    Tooltip = 0,
    Popup = 0,
    Modal = 0,
    ChildMenu = 0,
    DockNodeHost = 0,
}
---@enum ImGuiChildFlags
ImGuiChildFlags = {
    None = 0,
    Border = 0,
    AlwaysUseWindowPadding = 0,
    ResizeX = 0,
    ResizeY = 0,
    AutoResizeX = 0,
    AutoResizeY = 0,
    AlwaysAutoResize = 0,
    FrameStyle = 0,
    NavFlattened = 0,
}
---@enum ImGuiItemFlags
ImGuiItemFlags = {
    None = 0,
    NoTabStop = 0,
    NoNav = 0,
    NoNavDefaultFocus = 0,
    ButtonRepeat = 0,
    AutoClosePopups = 0,
}
---@enum ImGuiInputTextFlags
ImGuiInputTextFlags = {
    None = 0,
    CharsDecimal = 0,
    CharsHexadecimal = 0,
    CharsScientific = 0,
    CharsUppercase = 0,
    CharsNoBlank = 0,
    AllowTabInput = 0,
    EnterReturnsTrue = 0,
    EscapeClearsAll = 0,
    CtrlEnterForNewLine = 0,
    ReadOnly = 0,
    Password = 0,
    AlwaysOverwrite = 0,
    AutoSelectAll = 0,
    ParseEmptyRefVal = 0,
    DisplayEmptyRefVal = 0,
    NoHorizontalScroll = 0,
    NoUndoRedo = 0,
    CallbackCompletion = 0,
    CallbackHistory = 0,
    CallbackAlways = 0,
    CallbackCharFilter = 0,
    CallbackResize = 0,
    CallbackEdit = 0,
}
---@enum ImGuiTreeNodeFlags
ImGuiTreeNodeFlags = {
    None = 0,
    Selected = 0,
    Framed = 0,
    AllowOverlap = 0,
    NoTreePushOnOpen = 0,
    NoAutoOpenOnLog = 0,
    DefaultOpen = 0,
    OpenOnDoubleClick = 0,
    OpenOnArrow = 0,
    Leaf = 0,
    Bullet = 0,
    FramePadding = 0,
    SpanAvailWidth = 0,
    SpanFullWidth = 0,
    SpanTextWidth = 0,
    SpanAllColumns = 0,
    NavLeftJumpsBackHere = 0,
    CollapsingHeader = 0,
}
---@enum ImGuiPopupFlags
ImGuiPopupFlags = {
    None = 0,
    MouseButtonLeft = 0,
    MouseButtonRight = 0,
    MouseButtonMiddle = 0,
    MouseButtonMask_ = 0,
    MouseButtonDefault_ = 0,
    NoReopen = 0,
    NoOpenOverExistingPopup = 0,
    NoOpenOverItems = 0,
    AnyPopupId = 0,
    AnyPopupLevel = 0,
    AnyPopup = 0,
}
---@enum ImGuiSelectableFlags
ImGuiSelectableFlags = {
    None = 0,
    NoAutoClosePopups = 0,
    SpanAllColumns = 0,
    AllowDoubleClick = 0,
    Disabled = 0,
    AllowOverlap = 0,
}
---@enum ImGuiComboFlags
ImGuiComboFlags = {
    None = 0,
    PopupAlignLeft = 0,
    HeightSmall = 0,
    HeightRegular = 0,
    HeightLarge = 0,
    HeightLargest = 0,
    NoArrowButton = 0,
    NoPreview = 0,
    WidthFitPreview = 0,
    HeightMask_ = 0,
}
---@enum ImGuiTabBarFlags
ImGuiTabBarFlags = {
    None = 0,
    Reorderable = 0,
    AutoSelectNewTabs = 0,
    TabListPopupButton = 0,
    NoCloseWithMiddleMouseButton = 0,
    NoTabListScrollingButtons = 0,
    NoTooltip = 0,
    DrawSelectedOverline = 0,
    FittingPolicyResizeDown = 0,
    FittingPolicyScroll = 0,
    FittingPolicyMask_ = 0,
    FittingPolicyDefault_ = 0,
}
---@enum ImGuiTabItemFlags
ImGuiTabItemFlags = {
    None = 0,
    UnsavedDocument = 0,
    SetSelected = 0,
    NoCloseWithMiddleMouseButton = 0,
    NoPushId = 0,
    NoTooltip = 0,
    NoReorder = 0,
    Leading = 0,
    Trailing = 0,
    NoAssumedClosure = 0,
}
---@enum ImGuiFocusedFlags
ImGuiFocusedFlags = {
    None = 0,
    ChildWindows = 0,
    RootWindow = 0,
    AnyWindow = 0,
    NoPopupHierarchy = 0,
    DockHierarchy = 0,
    RootAndChildWindows = 0,
}
---@enum ImGuiHoveredFlags
ImGuiHoveredFlags = {
    None = 0,
    ChildWindows = 0,
    RootWindow = 0,
    AnyWindow = 0,
    NoPopupHierarchy = 0,
    DockHierarchy = 0,
    AllowWhenBlockedByPopup = 0,
    AllowWhenBlockedByActiveItem = 0,
    AllowWhenOverlappedByItem = 0,
    AllowWhenOverlappedByWindow = 0,
    AllowWhenDisabled = 0,
    NoNavOverride = 0,
    AllowWhenOverlapped = 0,
    RectOnly = 0,
    RootAndChildWindows = 0,
    ForTooltip = 0,
    Stationary = 0,
    DelayNone = 0,
    DelayShort = 0,
    DelayNormal = 0,
    NoSharedDelay = 0,
}
---@enum ImGuiDockNodeFlags
ImGuiDockNodeFlags = {
    None = 0,
    KeepAliveOnly = 0,
    NoDockingOverCentralNode = 0,
    PassthruCentralNode = 0,
    NoDockingSplit = 0,
    NoResize = 0,
    AutoHideTabBar = 0,
    NoUndocking = 0,
}
---@enum ImGuiDragDropFlags
ImGuiDragDropFlags = {
    None = 0,
    SourceNoPreviewTooltip = 0,
    SourceNoDisableHover = 0,
    SourceNoHoldToOpenOthers = 0,
    SourceAllowNullID = 0,
    SourceExtern = 0,
    PayloadAutoExpire = 0,
    PayloadNoCrossContext = 0,
    PayloadNoCrossProcess = 0,
    AcceptBeforeDelivery = 0,
    AcceptNoDrawDefaultRect = 0,
    AcceptNoPreviewTooltip = 0,
    AcceptPeekOnly = 0,
}
---@enum ImGuiDataType
ImGuiDataType = {
    S8 = 0,
    U8 = 0,
    S16 = 0,
    U16 = 0,
    S32 = 0,
    U32 = 0,
    S64 = 0,
    U64 = 0,
    Float = 0,
    Double = 0,
    Bool = 0,
    COUNT = 0,
}
---@enum ImGuiDir
ImGuiDir = {
    None = 0,
    Left = 0,
    Right = 0,
    Up = 0,
    Down = 0,
    COUNT = 0,
}
---@enum ImGuiSortDirection
ImGuiSortDirection = {
    None = 0,
    Ascending = 0,
    Descending = 0,
}
---@enum ImGuiKey
ImGuiKey = {
    Tab = 0,
    LeftArrow = 0,
    RightArrow = 0,
    UpArrow = 0,
    DownArrow = 0,
    PageUp = 0,
    PageDown = 0,
    Home = 0,
    End = 0,
    Insert = 0,
    Delete = 0,
    Backspace = 0,
    Space = 0,
    Enter = 0,
    Escape = 0,
    LeftCtrl = 0,
    LeftShift = 0,
    LeftAlt = 0,
    LeftSuper = 0,
    RightCtrl = 0,
    RightShift = 0,
    RightAlt = 0,
    RightSuper = 0,
    Menu = 0,
    ["0"] = 0,
    ["1"] = 0,
    ["2"] = 0,
    ["3"] = 0,
    ["4"] = 0,
    ["5"] = 0,
    ["6"] = 0,
    ["7"] = 0,
    ["8"] = 0,
    ["9"] = 0,
    A = 0,
    B = 0,
    C = 0,
    D = 0,
    E = 0,
    F = 0,
    G = 0,
    H = 0,
    I = 0,
    J = 0,
    K = 0,
    L = 0,
    M = 0,
    N = 0,
    O = 0,
    P = 0,
    Q = 0,
    R = 0,
    S = 0,
    T = 0,
    U = 0,
    V = 0,
    W = 0,
    X = 0,
    Y = 0,
    Z = 0,
    F1 = 0,
    F2 = 0,
    F3 = 0,
    F4 = 0,
    F5 = 0,
    F6 = 0,
    F7 = 0,
    F8 = 0,
    F9 = 0,
    F10 = 0,
    F11 = 0,
    F12 = 0,
    F13 = 0,
    F14 = 0,
    F15 = 0,
    F16 = 0,
    F17 = 0,
    F18 = 0,
    F19 = 0,
    F20 = 0,
    F21 = 0,
    F22 = 0,
    F23 = 0,
    F24 = 0,
    Apostrophe = 0,
    Comma = 0,
    Minus = 0,
    Period = 0,
    Slash = 0,
    Semicolon = 0,
    Equal = 0,
    LeftBracket = 0,
    Backslash = 0,
    RightBracket = 0,
    GraveAccent = 0,
    CapsLock = 0,
    ScrollLock = 0,
    NumLock = 0,
    PrintScreen = 0,
    Pause = 0,
    Keypad0 = 0,
    Keypad1 = 0,
    Keypad2 = 0,
    Keypad3 = 0,
    Keypad4 = 0,
    Keypad5 = 0,
    Keypad6 = 0,
    Keypad7 = 0,
    Keypad8 = 0,
    Keypad9 = 0,
    KeypadDecimal = 0,
    KeypadDivide = 0,
    KeypadMultiply = 0,
    KeypadSubtract = 0,
    KeypadAdd = 0,
    KeypadEnter = 0,
    KeypadEqual = 0,
    AppBack = 0,
    AppForward = 0,
    GamepadStart = 0,
    GamepadBack = 0,
    GamepadFaceLeft = 0,
    GamepadFaceRight = 0,
    GamepadFaceUp = 0,
    GamepadFaceDown = 0,
    GamepadDpadLeft = 0,
    GamepadDpadRight = 0,
    GamepadDpadUp = 0,
    GamepadDpadDown = 0,
    GamepadL1 = 0,
    GamepadR1 = 0,
    GamepadL2 = 0,
    GamepadR2 = 0,
    GamepadL3 = 0,
    GamepadR3 = 0,
    GamepadLStickLeft = 0,
    GamepadLStickRight = 0,
    GamepadLStickUp = 0,
    GamepadLStickDown = 0,
    GamepadRStickLeft = 0,
    GamepadRStickRight = 0,
    GamepadRStickUp = 0,
    GamepadRStickDown = 0,
    MouseLeft = 0,
    MouseRight = 0,
    MouseMiddle = 0,
    MouseX1 = 0,
    MouseX2 = 0,
    MouseWheelX = 0,
    MouseWheelY = 0,
    ReservedForModCtrl = 0,
    ReservedForModShift = 0,
    ReservedForModAlt = 0,
    ReservedForModSuper = 0,
    COUNT = 0,
    None = 0,
    Ctrl = 0,
    Shift = 0,
    Alt = 0,
    Super = 0,
    Mask_ = 0,
    NamedKey_BEGIN = 0,
    NamedKey_END = 0,
    NamedKey_COUNT = 0,
    KeysData_SIZE = 0,
    KeysData_OFFSET = 0,
}
---@enum ImGuiInputFlags
ImGuiInputFlags = {
    None = 0,
    Repeat = 0,
    RouteActive = 0,
    RouteFocused = 0,
    RouteGlobal = 0,
    RouteAlways = 0,
    RouteOverFocused = 0,
    RouteOverActive = 0,
    RouteUnlessBgFocused = 0,
    RouteFromRootWindow = 0,
    Tooltip = 0,
}
---@enum ImGuiConfigFlags
ImGuiConfigFlags = {
    None = 0,
    NavEnableKeyboard = 0,
    NavEnableGamepad = 0,
    NavEnableSetMousePos = 0,
    NavNoCaptureKeyboard = 0,
    NoMouse = 0,
    NoMouseCursorChange = 0,
    NoKeyboard = 0,
    DockingEnable = 0,
    ViewportsEnable = 0,
    DpiEnableScaleViewports = 0,
    DpiEnableScaleFonts = 0,
    IsSRGB = 0,
    IsTouchScreen = 0,
}
---@enum ImGuiBackendFlags
ImGuiBackendFlags = {
    None = 0,
    HasGamepad = 0,
    HasMouseCursors = 0,
    HasSetMousePos = 0,
    RendererHasVtxOffset = 0,
    PlatformHasViewports = 0,
    HasMouseHoveredViewport = 0,
    RendererHasViewports = 0,
}
---@enum ImGuiCol
ImGuiCol = {
    Text = 0,
    TextDisabled = 0,
    WindowBg = 0,
    ChildBg = 0,
    PopupBg = 0,
    Border = 0,
    BorderShadow = 0,
    FrameBg = 0,
    FrameBgHovered = 0,
    FrameBgActive = 0,
    TitleBg = 0,
    TitleBgActive = 0,
    TitleBgCollapsed = 0,
    MenuBarBg = 0,
    ScrollbarBg = 0,
    ScrollbarGrab = 0,
    ScrollbarGrabHovered = 0,
    ScrollbarGrabActive = 0,
    CheckMark = 0,
    SliderGrab = 0,
    SliderGrabActive = 0,
    Button = 0,
    ButtonHovered = 0,
    ButtonActive = 0,
    Header = 0,
    HeaderHovered = 0,
    HeaderActive = 0,
    Separator = 0,
    SeparatorHovered = 0,
    SeparatorActive = 0,
    ResizeGrip = 0,
    ResizeGripHovered = 0,
    ResizeGripActive = 0,
    TabHovered = 0,
    Tab = 0,
    TabSelected = 0,
    TabSelectedOverline = 0,
    TabDimmed = 0,
    TabDimmedSelected = 0,
    TabDimmedSelectedOverline = 0,
    DockingPreview = 0,
    DockingEmptyBg = 0,
    PlotLines = 0,
    PlotLinesHovered = 0,
    PlotHistogram = 0,
    PlotHistogramHovered = 0,
    TableHeaderBg = 0,
    TableBorderStrong = 0,
    TableBorderLight = 0,
    TableRowBg = 0,
    TableRowBgAlt = 0,
    TextLink = 0,
    TextSelectedBg = 0,
    DragDropTarget = 0,
    NavHighlight = 0,
    NavWindowingHighlight = 0,
    NavWindowingDimBg = 0,
    ModalWindowDimBg = 0,
    COUNT = 0,
}
---@enum ImGuiStyleVar
ImGuiStyleVar = {
    Alpha = 0,
    DisabledAlpha = 0,
    WindowPadding = 0,
    WindowRounding = 0,
    WindowBorderSize = 0,
    WindowMinSize = 0,
    WindowTitleAlign = 0,
    ChildRounding = 0,
    ChildBorderSize = 0,
    PopupRounding = 0,
    PopupBorderSize = 0,
    FramePadding = 0,
    FrameRounding = 0,
    FrameBorderSize = 0,
    ItemSpacing = 0,
    ItemInnerSpacing = 0,
    IndentSpacing = 0,
    CellPadding = 0,
    ScrollbarSize = 0,
    ScrollbarRounding = 0,
    GrabMinSize = 0,
    GrabRounding = 0,
    TabRounding = 0,
    TabBorderSize = 0,
    TabBarBorderSize = 0,
    TabBarOverlineSize = 0,
    TableAngledHeadersAngle = 0,
    TableAngledHeadersTextAlign = 0,
    ButtonTextAlign = 0,
    SelectableTextAlign = 0,
    SeparatorTextBorderSize = 0,
    SeparatorTextAlign = 0,
    SeparatorTextPadding = 0,
    DockingSeparatorSize = 0,
    COUNT = 0,
}
---@enum ImGuiButtonFlags
ImGuiButtonFlags = {
    None = 0,
    MouseButtonLeft = 0,
    MouseButtonRight = 0,
    MouseButtonMiddle = 0,
    MouseButtonMask_ = 0,
}
---@enum ImGuiColorEditFlags
ImGuiColorEditFlags = {
    None = 0,
    NoAlpha = 0,
    NoPicker = 0,
    NoOptions = 0,
    NoSmallPreview = 0,
    NoInputs = 0,
    NoTooltip = 0,
    NoLabel = 0,
    NoSidePreview = 0,
    NoDragDrop = 0,
    NoBorder = 0,
    AlphaBar = 0,
    AlphaPreview = 0,
    AlphaPreviewHalf = 0,
    HDR = 0,
    DisplayRGB = 0,
    DisplayHSV = 0,
    DisplayHex = 0,
    Uint8 = 0,
    Float = 0,
    PickerHueBar = 0,
    PickerHueWheel = 0,
    InputRGB = 0,
    InputHSV = 0,
    DefaultOptions_ = 0,
    DisplayMask_ = 0,
    DataTypeMask_ = 0,
    PickerMask_ = 0,
    InputMask_ = 0,
}
---@enum ImGuiSliderFlags
ImGuiSliderFlags = {
    None = 0,
    AlwaysClamp = 0,
    Logarithmic = 0,
    NoRoundToFormat = 0,
    NoInput = 0,
    WrapAround = 0,
    InvalidMask_ = 0,
}
---@enum ImGuiMouseButton
ImGuiMouseButton = {
    Left = 0,
    Right = 0,
    Middle = 0,
    COUNT = 0,
}
---@enum ImGuiMouseCursor
ImGuiMouseCursor = {
    None = 0,
    Arrow = 0,
    TextInput = 0,
    ResizeAll = 0,
    ResizeNS = 0,
    ResizeEW = 0,
    ResizeNESW = 0,
    ResizeNWSE = 0,
    Hand = 0,
    NotAllowed = 0,
    COUNT = 0,
}
---@enum ImGuiMouseSource
ImGuiMouseSource = {
    Mouse = 0,
    TouchScreen = 0,
    Pen = 0,
    COUNT = 0,
}
---@enum ImGuiCond
ImGuiCond = {
    None = 0,
    Always = 0,
    Once = 0,
    FirstUseEver = 0,
    Appearing = 0,
}
