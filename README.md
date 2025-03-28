Conquer Space
======
Conquer Space is a grand strategy game set in the near future, with realistic orbital mechanics, and an emphasis
on economics and politics.

We will simulate near future technologies, economies, and politics with a high degree of detail.

Discord: https://discord.gg/h5r77Sa

The project is still in intensive development.

### Compiling
Clone the repo with
`git clone --depth 1 --recursive --shallow-submodules https://github.com/Conquer-Space/Conquer-Space`

#### Windows
Open the repository

`cd cqsp`

Make build files

`setup_env`

Compile:

`cmake --build build --config Release`

Or you can open the Visual Studio project in build, and compile.

#### Linux (Ubuntu)
Install the necessary libraries with

```shell
sudo apt-get update && sudo apt install -y libglfw3 libglfw3-dev libopenal-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev
```

Open the repository

`cd cqsp`

Make the build folder

`./setup_env.sh`

Create build files

`cmake --build build`

#### Mac
We are currently looking for developers on Mac! Please join the Discord to participate in the effort!
