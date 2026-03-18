if not global_state.selected_menu then
    global_state.selected_menu = ""
end

-- If we see a reload we should reload all our documents?
active_documents = {}
active_documents["exploration"] = rmlui.contexts[1]:LoadDocument('binaries/data/core/gui/mainscene/exploration_window.rml')

for i, v in pairs(active_documents) do
    if i == global_state.selected_menu then
        active_documents[i]:Show()
    end
end

function open_exploration_window(document)
    global_state.selected_menu = "exploration"
    active_documents["exploration"]:Show()
    active_documents["exploration"]:PullToFront()
end
