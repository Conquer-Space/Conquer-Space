Conquer Space
======

Discord: https://discord.gg/h5r77Sa

### Compiling
Get the repo with `git clone --depth 1 --recursive --shallow-submodules https://github.com/EhWhoAmI/cqsp`

#### Windows
Open the repository `cd cqsp`

Make build files `setup_env`

Compile: `cmake --build build --config Release`
Or you can open the Visual Studio files, and compile.

#### Linux (Ubuntu)
You'll need a few libraries, so `sudo apt-get update`, and `sudo apt install -y libglfw3 libglfw3-dev libopenal-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev`

Open the repository `cd cqsp`

Make the build folder `./setup_env.sh`

Create build files `cmake --build build`

#### Mac
Sorry, we don't have any mac developers, so if you are one, feel free to join us and the discord and help us!
