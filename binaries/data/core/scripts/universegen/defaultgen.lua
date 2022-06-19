local function place_factory_on_market(market, city, resource, amount)
    local factory = core.create_factory(city, recipes[resource], amount)
    core.attach_market(market, factory)
    core.set_power_consumption(factory, 1000, 60)
    -- Enable production next
    core.add_production(factory)
    return factory
end

generators:insert({
    name = "default",
    civ_init = function()
    end,
    universe_gen = function()
    end,
    planets = function(civ_id)
        -- TODO(EhWhoAmI): Add earth cities and earth
        local planet = core.get_civilization_planet(civ_id)
        if core.is_player(civ_id) then
            print("Initializing player")
            core.set_name(civ_id, "Player Civilization")
        end

        -- Add tech
        core.add_tech_progress(civ_id)

        core.complete_technology(civ_id, technologies["steel_forging"])
        core.complete_technology(civ_id, technologies["consumer_good_manufacturing"])
        core.complete_technology(civ_id, technologies["concrete_manufacturing"])
        core.complete_technology(civ_id, technologies["basic_mining"])
        core.research_technology(civ_id, technologies["blank_research"])
        core.add_potential_tech(civ_id, technologies["temp_research"])
    end
})
