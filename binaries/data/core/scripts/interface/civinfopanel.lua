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
    open = false,
    selected_price_good = core.GoodEntity.null,
    selected_market = 0,
}

function civinfopanel:goodcostpanel()
    if ImGui.Button("<") then
        self.selected_price_good = core.GoodEntity.null
    end
    if self.selected_price_good == core.GoodEntity.null then
        return
    end    
    -- Then show the good information
    local good_entity = core.good_entity_to_entity(self.selected_price_good)
    ImGui.Text(core.get_name(good_entity))
    print(tostring(self.selected_price_good))
    local market_history = core.get_market_history(self.selected_market)
    ImPlot.SetNextAxesToFit()
    if ImPlot.BeginPlot("Good Price") then
        ImPlot.SetupAxes("Tick", "Price")
        ImPlot.PlotLine("Price", market_history.price_history[self.selected_price_good])
        ImPlot.EndPlot()
    end

    ImPlot.SetNextAxesToFit()
    if ImPlot.BeginPlot("Good Supply and Demand") then
        ImPlot.SetupAxes("Tick", "Supply")
        ImPlot.PlotLine("Supply", market_history.supply[self.selected_price_good])
        ImPlot.PlotLine("Demand", market_history.demand[self.selected_price_good])
        ImPlot.EndPlot()
    end
end

function civinfopanel:planetmarketinfopanel()
    if not ImGui.BeginTabBar("market_info_panel") then
        return
    end

    local markets = core.get_planetary_markets()
    for _, market in pairs(markets) do
        if ImGui.BeginTabItem(core.get_name(market)) then
            local market_comp = core.get_market(market)
            local market_history = core.get_market_history(market)
            ImGui.Text(core.get_name(market))
            ImGui.Text("Market GDP:".. core.to_human_string(market_comp.GDP))
            if ImGui.CollapsingHeader("Market GDP") then
                ImPlot.SetNextAxesToFit()
                if ImPlot.BeginPlot("Market GDP") then
                    ImPlot.SetupAxes("Tick", "GDP")
                    ImPlot.PlotLine("GDP", market_history.gdp)
                    ImPlot.EndPlot()
                end
            end
            local selected = client.SelectableMarketInformationTable(market)
            if selected ~= core.GoodEntity.null then
                self.selected_price_good = selected
                self.selected_market = market
                -- now display path or something
            end
            ImGui.EndTabItem()
        end
    end
    ImGui.EndTabBar()
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
            -- Make table for more random stuff
            if ImGui.BeginTable("Province table", 5) then
                ImGui.TableSetupColumn("Province Name")
                ImGui.TableSetupColumn("Deficit")
                ImGui.TableSetupColumn("Cumulative Market Deficit")
                ImGui.TableSetupColumn("Trade Deficit")
                ImGui.TableSetupColumn("Cumulative Trade Deficit")
                ImGui.TableHeadersRow()
                for _, province in pairs(owned) do
                    ImGui.TableNextRow();
                    ImGui.TableSetColumnIndex(0)
                    ImGui.Text(core.get_name(province))
                    client.EntityTooltip(province)
                    ImGui.TableSetColumnIndex(1)
                    ImGui.Text(core.to_human_string(core.market_last_deficit(province)))
                    ImGui.TableSetColumnIndex(2)
                    ImGui.Text(core.to_human_string(core.market_deficit(province)))
                    ImGui.TableSetColumnIndex(3)
                    ImGui.Text(core.to_human_string(core.market_last_trade_deficit(province)))
                    ImGui.TableSetColumnIndex(4)
                    ImGui.Text(core.to_human_string(core.market_trade_deficit(province)))
                end
                ImGui.EndTable()
            end
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Budget") then
            ImGui.Text("Budget breakdown ToDo!")
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Markets") then
            if self.selected_price_good == core.GoodEntity.null then
                self:planetmarketinfopanel()
            else
                self:goodcostpanel()
            end
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Mission Queue") then
            local queue = core.get_mission_queue(player)
            for _, mission in pairs(queue) do
                ImGui.Text(core.get_name(mission))
            end
            ImGui.EndTabItem()
        end
        if ImGui.BeginTabItem("Research") then
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
        self:civinfopanel()
    end
    ImGui.End()
end

interfaces:insert(civinfopanel)
