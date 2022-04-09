local function get_planet_count()
    local planet_count = core.random_normal_int(6, 5)
    if planet_count < 0 then
        planet_count = 0
    end
    return planet_count
end

local function set_planet_orbit(planet, index, m, b)
    local distance = m * index + b

    -- Then add some variance to the value
    local variance = core.random_normal_int(-15, 15)
    distance = distance * (1 - variance/100)

    -- Set theta
    local degrees = core.random(0, 360)
    local ecc = 0

    core.set_orbit(planet, distance, degrees, ecc, 0)
end

local function place_factory_on_market(market, city, resource, amount)
    local factory = core.create_factory(city, recipes[resource], amount)
    core.attach_market(market, factory)
    core.set_power_consumption(factory, 1000, 60)
    -- Enable production next
    core.add_production(factory)
    return factory
end

local star_system_min = 0
local star_system_max = 1000
generators:insert({
    name = "default",
    civ_init = function()
    end,
    universe_gen = function()
        local star_system_count = 1000
        for _ = 1, star_system_count, 1 do
            print("Generating one")
            local sys = core.create_star_system()
            core.set_system_position(sys, core.random(star_system_min, star_system_max),
                                            core.random(star_system_min, star_system_max))

            -- Add star
            local star = core.add_star(sys)
            core.set_orbit(star, 0.001, 0, 0, 0)
            core.set_radius(star, core.random(1500000, 3000000))

            local planet_count = get_planet_count()
            local distance = core.random(1, 100)/100
            local first = core.random(1, 100)/100

            for planet_id = 0, planet_count, 1 do
                -- Create planets
                local planet_entity = core.add_planet(sys)
                -- Set orbits
                -- Distance can be modeled after a log graph, then convert to km because our regression was based off AU
                set_planet_orbit(planet_entity, planet_id, distance, first)

                local radius = core.random(1000, 30000)
                core.set_radius(planet_entity, radius)
                -- Set planet type
                -- Set planet terrain
                -- Esh, we'll deal with that later
                -- As you go further from the center, make it more likely that it is a gas planet
            end
        end

        for _, civ in pairs(civilizations) do
            print("Making civilization "..civ)
            -- Set planets
            local sys = core.create_star_system()
            core.set_system_position(sys, core.random(star_system_min, star_system_max),
                                            core.random(star_system_min, star_system_max))

            -- Add star
            local star = core.add_star(sys)
            core.set_orbit(star, 0.001, 0, 0, 0)
            core.set_radius(star, core.random(1500000, 3000000))

            local planet_count = core.random_normal_int(7, 2)
            if planet_count < 0 then
                planet_count = 0
            end
            local distance = core.random(1, 100)/100
            local first = core.random(1, 100)/100
            for planet_id = 0, planet_count, 1 do
                -- Create planets
                local planet_entity = core.add_planet(sys)

                -- Set orbits
                -- Distance can be modeled after a log graph, then convert to km because our regression was based off AU
                set_planet_orbit(planet_entity, planet_id, distance, first)
                core.set_name(planet_entity, ""..planet_entity)
                local radius = core.random(1000, 30000)
                core.set_radius(planet_entity, radius)
                -- Set planet type
                -- Set planet terrain
                -- Esh, we'll deal with that later
                -- As you go further from the center, make it more likely that it is a gas planet
                if planet_id == 2 then
                    -- Set as civ home planet
                    core.set_civilization_planet(civ, planet_entity)
                    core.set_name(planet_entity, "Earth")
                    --core.set_name(core.create_ship(civ, planet_entity, sys), "Bubbles")
                    core.create_terrain(planet_entity, core.random(0, 10000000), terrain_colors["earth"])
                else
                    -- Choose a random terrain
                    local keys = {"moon", "terrain1", "terrain2"}
                    local color = terrain_colors[keys[core.random(1, (#keys))]]
                    core.create_terrain(planet_entity, core.random(0, 10000000), color)
                end
            end
        end
    end,
    planets = function(civ_id)
        -- TODO(EhWhoAmI): Add earth cities and earth
        local planet = core.get_civilization_planet(civ_id)
        if core.is_player(civ_id) then
            print("Initializing player")
            core.set_name(civ_id, "Player Civilization")
        end

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
        end
    end
})
