function open_exploration_window(document)
    local doc = document.context:LoadDocument('binaries/data/core/gui/mainscene/exploration_window.rml')
    if doc then
        doc:Show()
    end
    return doc
end
