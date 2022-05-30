Architecture
===============
Conquer Space's core is in [ECS](https://github.com/SanderMertens/ecs-faq), and runs on [entt](https://github.com/skypjack/entt/).

## Project Architecture
Conquer Space consists of 3 libraries and 1 project.

### cqsp-core
This is where the universe, and all of the game components are. In theory, this library should be standalone and should not need the other 2 libraries to compile.

### cqsp-engine
Conquer Space's game engine is built from ground up.

This provides lots of functions to simplify the rendering of objects, though lots of functionality is still greatly desired.

Asset loading takes place in this stage.

### cqsp-client
This is where all the UI and rendering takes place.

We use [RmlUi](https://github.com/mikke89/RmlUi) for our main UI, and [ImGui](https://github.com/ocornut/imgui) to create our UI for debugging or prototyping.

### cqsp-main
Just a main function to run the application.

## Game Architecture
The main game loop takes place in `src/common/simulation.h`.

A new system is added in the constructor of the simulation.
