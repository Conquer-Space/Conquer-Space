if not selected_menu then
    selected_menu = ""
end

-- If we see a reload we should reload all our documents?
active_documents = {}
print(tostring(rmlui.contexts))
for k,v in pairs(rmlui.contexts) do
    print(tostring(k).." = "..tostring(v))
end
-- I forgot lua starts indices by 1 lmfao
active_documents["exploration"] = rmlui.contexts[1]:LoadDocument('binaries/data/core/gui/mainscene/exploration_window.rml')
print("Loaded document")

for i in pairs(active_documents) do
    print(i)
    if i == selected_menu then
        active_documents[i]:Show()
        print("Showing...")
    end
end

function open_exploration_window(document)
    selected_menu = "exploration"
    active_documents["exploration"]:Show()
    active_documents["exploration"]:PullToFront()
end
