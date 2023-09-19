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
- Linux Distribution ([WSL](https://learn.microsoft.com/en-us/windows/wsl/install) can be used if running on Windows)
- SDL2
- SDL2_image
- "sand" binary as downloaded from the "releases" tab.

### Installation

SDL2 is required for sand-sim to run. To install the library system-wide on Linux/Unix,
run the following command:

```bash
sudo apt-get install libsdl2-2.0-0
```

SDL2_image is also required, which can be installed similarly:

```bash
sudo apt-get install libsdl2-image-2.0-0
```

Once SDL2 is installed, sand-sim can be run like so:

```bash
./sand
```

### Controls

Use the left mouse button the generate sand tiles into the world.

The element type may be changed using the keyboard. The key controls are:

1 - Sand
2 - Water
3 - Wood
4 - Steam
(More elements and interactions to come in future versions!)

The panel in the topleft represents your currently selected element.

## Building From Source

Compiling sand-sim requires the development versions of SDL2 and SDL2_image, which can be installed
on Linux/Unix with the commands:

```bash
sudo apt-get install libsdl2-2.0-0-dev
sudo apt-get install libsdl2-image-2.0-0-dev
```

Download and extract a copy of the source code from the "releases" tab.

Inside the root directory of the repository, the provided Makefile can be used to compile sand-sim.
Compilation requires clang, which can be installed with the command:

```bash
sudo apt-get install clang
```

Once clang is installed, sand-sim can be built with the command:

```bash
make sand
```

This will produce a binary called "sand" which can be executed to run the program.

## Source File Organization

- "sandbox.h" - Contains functions for sandbox simulation logic.
- "gui.h" - Contains structures and functions for displaying a sandbox using SDL2.
- "assets/" - Directory containing all visual assets.
- "test.c" - Debugging code.