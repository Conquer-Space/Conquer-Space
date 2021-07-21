-- This script is run for every civilization
-- Inputs: civilization_id: the entt id of the civilization
print("Initializing civ ".. civilization_id)
-- Set planet information and stuff

local planet = get_civilization_planet(civilization_id)

if is_player(civilization_id) then
    print("Is player")
end

-- Add cities
add_planet_habitation(planet)

local market = create_market()

place_market(market, planet)

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
    -- Attach these stuff to market
    attach_market(market, create_factory(city, recipes["steel_forging"], 10))
    -- Add various factories
    attach_market(market, create_factory(city, recipes["consumer_good_manufacturing"], 10))
    attach_market(market, create_mine(city, goods["copper"], 10, 1))
    attach_market(market, create_mine(city, goods["aluminium"], 10, 1))
    attach_market(market, create_mine(city, goods["iron"], 70, 1))
end
