local civinfopanel = {
    open = true
}

function civinfopanel:do_ui()
    ImGui.SetNextWindowPos(0, 0, ImGuiCond.Always)
    if self.open then
        local disp_x, disp_y = ImGui.GetDisplaySize()
        print(disp_x)
        ImGui.SetNextWindowSize(disp_x * 0.2, disp_y * 0.4, ImGuiCond.Appearing)
    else
        ImGui.SetNextWindowSize(-1, -1)
    end
    Imgui.Begin("Civilization Window")
    if ImGui.Button("Show/Hide Civilization") then
    self.open = not self.open
    end
    if self.open then
        -- Then civ info panel
    end
    ImGui.End()
end

interfaces:insert(civinfopanel)