-- We need some event listener
CountrySelectionWindow = CountrySelectionWindow or {}

function CountrySelectionWindow.OnProvinceSelect(document)
    -- Also get the current document to actually set it
    -- local country = core.get_name(country)
    local name = core.get_name(selection.get_selected_country())
    -- Now we should update everything...
    -- how do we plumb this
    document:GetElementById('country-name').inner_rml = name
    -- set the button rml
    document:GetElementById('start-game'):SetClass('disabled', false)
    document:GetElementById('start-game'):SetClass('enabled', true)
end
