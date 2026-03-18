-- now get the central div and do stuff
ExplorationWindow = ExplorationWindow or {}

local planet_model = {
    name = "",
    orbit = core.Orbit:new()
}

if not global_state.exploration_window then
    global_state.exploration_window = {}
    global_state.exploration_window.data_model = rmlui.contexts[1]:OpenDataModel("planet_model", planet_model)
end

function ExplorationWindow.RefreshPlanet(planet)
    global_state.exploration_window.data_model.orbit = core.get_orbit(planet)
    global_state.exploration_window.data_model.name = core.get_name(planet)
end

-- now let's make a context or something
function ExplorationWindow.OnLoad(document)
    planets = core.get_planets()
    planet_str = ""
    local planet_button = document:GetElementById('planet_button')
    for i, planet in ipairs(planets) do
        planet_str = planet_str..core.get_name(planet).." "
        local panel_button = planet_button:Clone()
        panel_button.inner_rml = core.get_name(planet)
        panel_button.style['display'] = 'inline'
        panel_button:SetAttribute('onclick', 'ExplorationWindow.RefreshPlanet('..planet..')')
        document:GetElementById('planet_panel'):AppendChild(panel_button)
    end
end
