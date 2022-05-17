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

        -- Add cities
        core.add_planet_habitation(planet)
        local market = core.create_market()
        core.place_market(market, planet)

        core.attach_market(market, civ_id)

        -- Seed market resources
        --add_resource(market, goods["steel"], 50000)
        --add_resource(market, goods["copper"], 50000)

        core.set_resource(planet, goods["copper"], 10)

        -- Add city
        -- TODO(EhWhoAmI): Create more complex economy, with cities specializing in stuff
        -- TODO(EhWhoAmI): Make slider for configuring the amount of cities and stuff
        local city_count = 1
        for _ = 1, city_count, 1 do
            local city = core.add_planet_settlement(planet, core.random(-90, 90), core.random(-180, 180))
            -- Get random.random city name
            -- Set ownership
            core.set_name(city, core.get_random_name("Town Names", "1"))
            core.set_owner(city, civ_id)
            local pop_unit = core.add_population_segment(city, core.random_normal_int(50000000, 2000000)) -- 100 million
            core.add_cash(pop_unit, 1000) -- 1 billion
            core.attach_market(market, pop_unit)
            -- Add industry
            core.create_industries(city)
            -- Add various factories
            core.create_commercial_area(city)

            -- According to syspopulation.cpp, consumption of resources is about 0.09261 kg per day, multiply that by 25
            -- for the tick rate, we get 0.09261*25 = 2.31525
            -- 95% of cities should have a population under 53919928, so we'd calculate for that
            -- 53919928 * 2.31525 / 2 (for the recipe) = 62419056.651

            place_factory_on_market(market, city, "consumer_good_manufacturing", 100)

            -- Steel needed
            place_factory_on_market(market, city, "steel_forging", 100)
            -- place_factory_on_market(market, city, "concrete_manufacturing", 300)
            core.attach_market(market, core.create_mine(city, goods["copper"], 1, 200))
            -- attach_market(market, create_mine(city, goods["aluminium"], 1, 10000))
            -- attach_market(market, create_mine(city, goods["stone"], 1, 10000))
            core.attach_market(market, core.create_mine(city, goods["iron"], 1, 300))
            -- attach_market(market, create_mine(city, goods["oil"], 1, 50000))
            -- add_power_plant(city, 1000)
            -- Add farms
            core.attach_market(market, core.create_farm(city, goods["food"], 1, 300))
            local lab = core.create_lab()
            print(fields["geometry"])
            core.add_science(lab, fields["geometry"], 10)
            core.add_industry(city, lab)
        end
    end
})
