--[[
Conquer Space
Copyright (C) 2021-2023 Conquer Space

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
--]]
local civinfopanel = {
    open = false
}

function civinfopanel:planetmarketinfopanel()
    
end
function civinfopanel:civinfopanel()
    local player = core.get_player()
    ImGui.Text(core.get_name(player))
    client.EntityTooltip(player)
    ImGui.Separator()
    ImGui.Text("Capital City: " ..core.get_name(core.get_capital_city(player)))
    client.EntityTooltip(core.get_capital_city(player))
    ImGui.Text("Currency Reserves: "..core.to_human_string(core.get_balance(player)))
    if ImGui.BeginTabBar("civ_info_window") then
        if ImGui.BeginTabItem("City Information") then
            ImGui.Text("Owned Cities")
            ImGui.Separator()
            ImGui.BeginChild("ownedcitiespanel")
            -- Loop through cities and see what happens
            local gov = core.get_governed(player)
            for _, city in pairs(gov) do
                ImGui.Text(core.get_name(city))
                client.EntityTooltip(city)
            end
            ImGui.EndChild()
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Province Information") then
            local owned = core.get_owned_cities(player)
            for _, province in pairs(owned) do
                ImGui.Selectable(core.get_name(province))
                client.EntityTooltip(province)
            end
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Market Information") then
            client.MarketInformationTable(player)
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Budget") then
            ImGui.Text("Budget breakdown ToDo!")
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Markets") then
            ImGui.EndTabItem()
        end
        ImGui.EndTabBar()
    end
end

function civinfopanel:do_ui()
    ImGui.SetNextWindowPos(0, 0, ImGuiCond.Always)
    if self.open then
        local disp_x, disp_y = ImGui.GetDisplaySize()
        ImGui.SetNextWindowSize(disp_x * 0.2, disp_y * 0.4, ImGuiCond.Appearing)
    else
        ImGui.SetNextWindowSize(-1, -1)
    end
    ImGui.Begin("Civilization Window")
    if ImGui.Button("Show/Hide Civilization") then
    self.open = not self.open
    end
    if self.open then
        -- Then civ info panel
        self.civinfopanel()
    end
    ImGui.End()
end

interfaces:insert(civinfopanel)
