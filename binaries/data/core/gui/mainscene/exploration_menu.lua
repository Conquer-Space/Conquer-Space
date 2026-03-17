-- now get the central div and do stuff
ExplorationWindow = ExplorationWindow or {}
function ExplorationWindow.OnLoad(document)
    planets = core.get_planets()
    planet_str = ""
    for i, planet in ipairs(planets) do
        planet_str = planet_str..core.get_name(planet).." "
        -- somehow sort by radius
    end
    document:GetElementById('planet_panel').inner_rml = planet_str
    document:GetElementById('planet_panel'):AppendChild(document:GetElementById('planet_panel'):Clone())
end
