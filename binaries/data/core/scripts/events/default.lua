local starting_event = {
    chain = 0
}

function starting_event:on_tick()
    if date == 0 then
        local home_name = get_name(get_civilization_planet(get_player()))
        push_event(get_player(), {
            id = "rocket-event",
            image = "rocket-event",
            title = "Our Future is in the Stars",
            content = "Since antiquity, our ancestors have always looked to "..
            "the stars for guidance, inspiration, and hope. But now, "..
            "we have finally reached the technological advancement to touch the stars. "..
            "This promises to bring untold changes to our culture, economy, and technology. "..
            "This will change our future forever. For the first time in history, our people "..
            "will no longer solely reside on "..home_name..", and will be an interplanetary civilization.",
            actions = {{
                name = "Begin!",
                tooltip = "Starts the \"Conquer The Stars\" event chain",
                action = function()
                    chain = 1
                end
            }}
        })
    end
    if chain == 1 and date > 100 then
        push_event(get_player(), {
            id = "rocket-event",
            image = "rocket-event",
            title = "Conquer The Stars 1",
            content = "We need the technology to conquer the stars. First, let's research stuff.",
            actions = {{
                name = "Begin!",
                tooltip = "Starts the \"Conquer The Stars\" event chain",
                action = function()
                    chain = 2
                end
            }}
        })
    end
end

events:insert(starting_event)
