local test_interface = {
    open = true
}

function test_interface:do_ui()
    local should_draw = false
    self.open, should_draw = ImGui.Begin("Test ui", self.open, 0)
    if should_draw then
        ImGui.Text("Testing hehe ")
        ImGui.Separator()
        ImGui.Text("This is more text")
        ImGui.Text("This is the longest text that you can have")
        ImGui.ProgressBar(0.5, 100, 25, "Loading Failed. Sike. - 50%")
        ImGui.End()
    end
end

interfaces:insert(test_interface)
