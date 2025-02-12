local test_interface = {
    open = true
}

function test_interface:do_ui()
    if ImGui.Begin("Test ui", self.open, 0) then
        ImGui.Text("Testing hehe")
        ImGui.Separator()
        ImGui.Text("This is more text")
        ImGui.Text("This is the longest text that you can have")
        ImGui.End()
    end
end

interfaces:insert(test_interface)
