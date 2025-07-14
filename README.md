# Sand Simulation (sand-sim)
An implementation of a sandbox cellular automata.

In the sandbox, a tile interacts with the tiles immediately surrounding it, depending on what element the tile is.

Sand falls, water flows, steam rises, wood burns, etc!

Written in C using [SDL2](https://www.libsdl.org/).

Personal project by Daniel Galvan.

![Demo of Program](src/assets/demo/demo.gif)

This project, while presentable in its current form, is planned to be further extended with additional customization options, elements, and toy features.


## Usage
### Prerequisites

If running on a Linux Distribution:
- Contents of "sand-sim" directory as downloaded from the "releases" tab.
- SDL2
- SDL2_image

If running on Windows:
- Contents of "sand-sim-win" directory as downloaded from the "releases" tab.

Alternatively, [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) can be used if running on Windows to use
the Linux version.

### Installation

sand-sim must be executed with its "assets" directory in the same location as the executable.

SDL2 is required for sand-sim to run.

For Windows users, this means the provided "SDL2.dll" and "SDL2_image.dll" must be present in the same directory as "sand.exe".
(This is already set up correctly in the "sandwin" directory)

sand-sim can then be run using the "sand.exe" executable.

For Linux/Unix users, SDL2 must be installed system-wide. 

To do this, run the following command:

```bash
sudo apt-get install libsdl2-2.0-0
```

SDL2_image is also required, which can be installed similarly:

```bash
sudo apt-get install libsdl2-image-2.0-0
```

Once SDL2 is installed, sand-sim can be run using the provided binary:

```bash
./sand
```

### Controls

Use the left mouse button the generate sand tiles into the world.

The element type may be changed using the keyboard. The key controls are:

- 1 - Sand
- 2 - Water
- 3 - Wood
- 4 - Steam
(More elements and interactions to come in future versions!)

The panel in the topleft represents your currently selected element.

Press q to quit the app.

## Building From Source

Compiling either of sand-sim's versions is supported only for Linux/Unix environments.

Download and extract a copy of the source code from the "releases" tab.

Inside the "src" directory of the repository, the provided Makefile can be used to compile sand-sim.

### Linux/Unix

Compiling sand-sim's Linux/Unix version requires the development versions of SDL2 and SDL2_image, which can be installed
on Linux/Unix with the commands:

```bash
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
```

Compilation of the Linux/Unix version requires clang, which can be installed with the command:

```bash
sudo apt-get install clang
```

Once clang is installed, sand-sim can be built from the "src" directory with the command:

```bash
make sand
```

This will produce a binary called "sand" which can be executed to run the program.

### Windows

Compiling sand-sim's Windows version requires MingW64 and pkg-config, both of which can be installed with the commands:

```bash
sudo apt-get install mingw-w64
```

```bash
sudo apt-get install pkg-config
```

Once these are installed, sand-sim can be built from the "src" directory with the command:

```bash
make sandwin
```

The necessary SDL2 Windows compilation libraries are provided with the source of sand-sim.

## Source File Organization

- "sandbox.h" - Contains functions for sandbox simulation logic.
- "gui.h" - Contains structures and functions for displaying a sandbox using SDL2.
- "assets/" - Directory containing all visual assets.
- "test.c" - Debugging code.
