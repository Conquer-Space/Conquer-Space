-- now get the central div and do stuff
CivilizationWindow = CivilizationWindow or {}


if not global_state.civilization_window then
    global_state.civilization_window = {}
end

function CivilizationWindow.OnLoad(document)
-- now compute gdp and other stats?
    local gdp_child = document:GetElementById("gdp_text")
    gdp_child.inner_rml = core.to_human_string(core.get_gdp(core.get_player()))
    local population_child = document:GetElementById("population_text")
    population_child.inner_rml = core.to_human_string(core.get_national_population(core.get_player()))
    local child = document:GetElementById("budget_text")
    child.inner_rml = core.to_human_string(core.get_balance(core.get_player()))
end

function CivilizationWindow.OnTick(document)
end
