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
    variance = random_normal_int(-15, 15)
    distance = distance * (1 - variance/100)

    -- Set theta
    degrees = random(0, 360)
    ecc = 0

    set_orbit(planet, distance, degrees, ecc, 0)
end
print("Generating")
generators:insert({
    name = "default",
    civ_init = function()
    end,
    universe_gen = function()
        print("Generating star system")
        for x = 1, 1000, 1 do
            local sys = create_star_system()
        
            -- Set orbit
            local star_count = 1
        
            -- Add star
            local star = add_star(sys)
            set_orbit(star, 0.001, 0, 0, 0)
            set_radius(star, random(1500000, 3000000))
        
            local planet_count = get_planet_count()
            local distance = random(1, 100)/100
            local first = random(1, 100)/100
            for planet_id = 0, planet_count, 1 do
                -- Create planets
                planet_entity = add_planet(sys)
        
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
            print("Making civilization")
            -- Set planets
            local sys = create_star_system()
        
            -- Set orbit
            local star_count = 1
        
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
                planet_entity = add_planet(sys)
        
                -- Set orbits
                -- Distance can be modeled after a log graph, then convert to km because our regression was based off AU
                set_planet_orbit(planet_entity, planet_id, distance, first)
                set_name(planet_entity, ""..planet_entity)
                local radius = random(1000, 30000)
                set_radius(planet_entity, radius)
                create_terrain(planet_entity, random(0, 10000000))
                -- Set planet type
                -- Set planet terrain
                -- Esh, we'll deal with that later
                -- As you go further from the center, make it more likely that it is a gas planet
        
                if planet_id == 2 then
                    -- Set as civ home planet
                    set_civilization_planet(civ, planet_entity)
                    set_name(planet_entity, "Earting")
                    set_name(create_ship(civ, planet_entity, sys), "Earting Ship")
                end
            end
        end
    end,
    planets = function(civ_id)
        local planet = get_civilization_planet(civ_id)
        if is_player(civ_id) then
            print("Is player")
        end

        -- Add cities
        add_planet_habitation(planet)
        market = create_market()
        add_resource(market, goods["steel"], 50000)
        add_resource(market, goods["copper"], 50000)

        -- Add city
        local city_count = random(30, 50)
        for index = 0, city_count, 1 do
            local city = add_planet_settlement(planet)
            set_name(city, "City ".. index)
            local pop_unit = add_population_segment(city, 100000000) -- 100 million
            -- Add industry
            create_industries(city)
            attach_market(market, create_factory(city, recipes["steel_forging"], 10))
            -- Add various factories
            attach_market(market, create_factory(city, recipes["consumer_good_manufacturing"], 9))
            attach_market(market, create_mine(city, goods["copper"], 10, 1))
            attach_market(market, create_mine(city, goods["aluminium"], 10, 1))
            attach_market(market, create_mine(city, goods["iron"], 70, 1))
        end
    end
})
