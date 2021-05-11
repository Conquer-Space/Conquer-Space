
function get_planet_count()
    local planet_count = random_normal_int(6, 5)
    if planet_count < 0 then
        planet_count = 0
    end
    return planet_count
end

function set_planet_orbit(planet, index, m, b)
    local distance = m * index + b

    -- Then add some variance to the value
    variance = random_normal_int(-15, 15)
    distance = distance * (1 - variance/100)

    -- Set theta
    degrees = random(0, 360)
    ecc = 0

    set_orbit(planet, distance, degrees, ecc, 0)
end

for x = 1, 1000, 1 do
    local sys = create_star_system()

    -- Set orbit
    local star_count = 1

    -- Add star
    local star = add_star(sys)
    set_orbit(star, 0, 0, 0, 0)
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
    -- Set planets
    local sys = create_star_system()

    -- Set orbit
    local star_count = 1

    -- Add star
    local star = add_star(sys)
    set_orbit(star, 0, 0, 0, 0)
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

        local radius = random(1000, 30000)
        set_radius(planet_entity, radius)
        -- Set planet type
        -- Set planet terrain
        -- Esh, we'll deal with that later
        -- As you go further from the center, make it more likely that it is a gas planet

        if planet_id == 2 then

            -- Set as civ home planet
            set_civilization_planet(civ, planet_entity)
        end
    end
end