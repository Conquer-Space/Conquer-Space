-- This script is run for every civilization
-- Inputs: civilization_id: the entt id of the civilization
print("Initializing civ ".. civilization_id)
-- Set planet information and stuff

local planet = get_civilization_planet(civilization_id)

if is_player(civilization_id) then
    print("Is player, do something")
end

-- Add cities
add_planet_habitation(planet)

-- Add city
local city_count = random(30, 50)
for index = 0, city_count, 1 do
    local city = add_planet_settlement(planet)
    set_name(city, "City ".. index)
    local pop_unit = add_population_segment(city, 100000000) -- 100 million
    -- Add industry
    create_industries(city)
    create_factory(city, recipes["steel_forging"])
    -- Add various factories
    create_factory(city, recipes["consumer_good_manufacturing"])
    create_mine(city, goods["iron"], 10)
end
