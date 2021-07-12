local test_event = {
    evented = false,
    choice = -1
}

function test_event:on_tick()
    if date % 30 == 0 and not self.evented then
        event_player({
            id = "galactic_empire_event",
            image = "eventimage",
            title = "Declaration of a New Order",
            content = "*In order to ensure our security and continuing stability, the Republic will be reorganized into the first Galactic Empire, for a safe and secure society, which I assure you will last for ten thousand years!* \n\nWith these words, Supreme Chancellor Palpatine became Emperor Palpatine, and abolished the Galactic Republic. The Jedi Order was abolished, and members of it were brutally massacred. The Galactic Senate unanimously approved this transition, allowing for the consolidation of power to one individual, the Galactic Emperor. How this will affect the galaxy at large, is yet to be seen.",
            actions = {
                {
                    name = "This is how liberty dies ... with thunderous applause",
                    tooltip = "The Galactic Rebellion is founded",
                    action = function()
                        self.choice = 0
                    end
                },
                {
                    name = "Safety, Security, Justice, and Peace!",
                    tooltip = "The Galactic Empire gains your support",
                    action = function()
                        self.choice = 1
                    end
                }
            }
        })
        self.evented = true
    end
    if date % 30 == 0 and self.choice > -1 then
        print("Choice")
        event_player({
            id = "republic_event",
            image = "eventimage2",
            title = "The Alliance to Restore the Republic",
            content = "*They've no idea we're coming. They've no reason to expect us. If we can make it to the ground, we'll take the next chance, and the next, on and on until we win, or the chances are spent.*\n\nAfter years of insurgency and small guerilla attacks, the first large fleet operations of the Rebel Alliance took place off the surface of Scarif. Scarif contained valuable information pertaining to the Galactic Empire’s weapons project. How this would affect the Empire as a whole is yet to be seen.",
            actions = {
                {
                    name = "The Galactic Civil War has Begun",
                    action = function()
                        self.choice = -1
                    end
                }
            }
        })
    end
end
events:insert(test_event)
