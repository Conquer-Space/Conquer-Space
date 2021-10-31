local function get_planet_count()
    local planet_count = random_normal_int(6, 5)
    if planet_count < 0 then
        planet_count = 0
    end
    return planet_count
end

local function set_planet_orbit(planet, index, m, b)
    local distance = m * index + b

    -- Then add some variance to the value
    local variance = random_normal_int(-15, 15)
    distance = distance * (1 - variance/100)

    -- Set theta
    local degrees = random(0, 360)
    local ecc = 0

    set_orbit(planet, distance, degrees, ecc, 0)
end

local function place_factory_on_market(market, city, resource, amount)
    local factory = create_factory(city, recipes[resource], amount)
    attach_market(market, factory)
    return factory
end

local star_system_min = 0
local star_system_max = 1000
generators:insert({
    name = "default",
    civ_init = function()
        local player = add_civilization()
        local player_species = create_species("Human")
        set_player(player)
        set_civilization_species(player, player_species)
        set_species_food(player_species, create_food())

        local civ_count = 9
        for _ = 1, civ_count, 1 do
            local civ = add_civilization()
            local civ_species = create_species("Not Human")
            set_civilization_species(civ, civ_species)
            set_species_food(civ_species, create_food())
        end
    end,
    universe_gen = function()
        local star_system_count = 1000
        for _ = 1, star_system_count, 1 do
            local sys = create_star_system()
            set_system_position(sys, random(star_system_min, star_system_max), random(star_system_min, star_system_max))

            -- Add star
            local star = add_star(sys)
            set_orbit(star, 0.001, 0, 0, 0)
            set_radius(star, random(1500000, 3000000))

            local planet_count = get_planet_count()
            local distance = random(1, 100)/100
            local first = random(1, 100)/100
            for planet_id = 0, planet_count, 1 do
                -- Create planets
                local planet_entity = add_planet(sys)

                -- Set orbits
                -- Distance can be modeled after a log graph, then convert to km because our regression was based off AU
                set_planet_orbit(planet_entity, planet_id, distance, first)

                local radius = random(1000, 30000)
                set_radius(planet_entity, radius)
                -- Set planet type
                -- Set planet terrain
                -- Esh, we'll deal with that later
                -- As you go further from the center, make it more likely that it is a gas planet
            end
        end

        for _, civ in pairs(civilizations) do
            print("Making civilization "..civ)
            -- Set planets
            local sys = create_star_system()
            set_system_position(sys, random(star_system_min, star_system_max), random(star_system_min, star_system_max))

            -- Add star
            local star = add_star(sys)
            set_orbit(star, 0.001, 0, 0, 0)
            set_radius(star, random(1500000, 3000000))

            local planet_count = random_normal_int(7, 2)
            if planet_count < 0 then
                planet_count = 0
            end
            local distance = random(1, 100)/100
            local first = random(1, 100)/100
            for planet_id = 0, planet_count, 1 do
                -- Create planets
                local planet_entity = add_planet(sys)

                -- Set orbits
                -- Distance can be modeled after a log graph, then convert to km because our regression was based off AU
                set_planet_orbit(planet_entity, planet_id, distance, first)
                set_name(planet_entity, ""..planet_entity)
                local radius = random(1000, 30000)
                set_radius(planet_entity, radius)
                -- Set planet type
                -- Set planet terrain
                -- Esh, we'll deal with that later
                -- As you go further from the center, make it more likely that it is a gas planet
                if planet_id == 2 then
                    -- Set as civ home planet
                    set_civilization_planet(civ, planet_entity)
                    set_name(planet_entity, "Earth")
                    set_name(create_ship(civ, planet_entity, sys), "Bubbles")
                    create_terrain(planet_entity, random(0, 10000000), terrain_colors["earth"])
                else
                    -- Choose a random terrain
                    local keys = {"moon", "terrain1", "terrain2"}
                    local color = terrain_colors[keys[random(1, (#keys))]]
                    create_terrain(planet_entity, random(0, 10000000), color)
                end
            end
        end
    end,
    planets = function(civ_id)
        -- TODO(EhWhoAmI): Add earth cities and earth
        local planet = get_civilization_planet(civ_id)
        if is_player(civ_id) then
            print("Initializing player")
        end

        local species = get_civilization_species(civ_id)
        print(species)
        print(get_name(species))
        -- Add cities
        add_planet_habitation(planet)
        local market = create_market()
        place_market(market, planet)
        add_resource(market, goods["steel"], 50000)
        add_resource(market, goods["copper"], 50000)

        set_resource(planet, goods["copper"], 10)

        -- Add city
        -- TODO(EhWhoAmI): Create more complex economy, with cities specializing in stuff
        -- TODO(EhWhoAmI): Make slider for configuring the amount of cities and stuff
        local city_count = 2
        for index = 1, city_count, 1 do
            local city = add_planet_settlement(planet, random(-90, 90), random(-180, 180))
            set_name(city, "City ".. index)
            local pop_unit = add_population_segment(city, random_normal_int(50000000, 2000000), species) -- 100 million
            add_cash(pop_unit, 1000000000) -- 1 billion
            attach_market(market, pop_unit)
            -- Add industry
            create_industries(city)
            -- Add various factories
            create_commercial_area(city)

            -- According to syspopulation.cpp, consumption of resources is about 0.09261 kg per day, multiply that by 25
            -- for the tick rate, we get 0.09261*25 = 2.31525
            -- 95% of cities should have a population under 53919928, so we'd calculate for that
            -- 53919928 * 2.31525 / 2 (for the recipe) = 62419056.651

            place_factory_on_market(market, city, "consumer_good_manufacturing", 62419056.651)

            -- Steel neede
            place_factory_on_market(market, city, "steel_forging", 62419056.651)
            -- place_factory_on_market(market, city, "concrete_manufacturing", 300)
            attach_market(market, create_mine(city, goods["copper"], 1, 10000))
            attach_market(market, create_mine(city, goods["aluminium"], 1, 10000))
            attach_market(market, create_mine(city, goods["stone"], 1, 10000))
            attach_market(market, create_mine(city, goods["iron"], 1, 374514340))
        end
    end
})
