-- This script is run for every civilization
-- Inputs: civilization_id: the entt id of the civilization
print("Initializing civ ".. civilization_id)
-- Set planet information and stuff

local planet = get_civilization_planet(civilization_id)

if is_player(civilization_id) then
    print("Is player, do something")
end
