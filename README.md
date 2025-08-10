# Sand Simulation (sand-sim)
An implementation of a sandbox cellular automata.

In the sandbox, a tile particle interacts with the tiles immediately surrounding it, depending on what element the tile is.

Sand falls, water flows, steam rises, wood burns, fire extinguishes!

Written in C using [SDL2](https://www.libsdl.org/).

Personal project by Daniel Galvan.

![Demo of Program](assets/demo/demo.gif)

This project, while presentable in its current form, is planned to be further extended with additional customization options, elements, and compile targets.


## Usage
### Prerequisites

If running on a Debian-based Linux Distribution:
- Contents of "sand-sim" directory as downloaded from the "releases" tab.
- [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2)
- [SDL2_image](https://github.com/libsdl-org/SDL_image/tree/SDL2)

If running on Windows:
- Contents of "sand-sim-win" directory as downloaded from the "releases" tab.

Alternatively, [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) can be used if running on Windows to use the Linux version.

### Installation

sand-sim must be executed with its "assets" directory in the same location as the executable.

SDL2 is required for sand-sim to run.

For Windows users, this means the provided "SDL2.dll" and "SDL2_image.dll" must be present in the same directory as "sand-sim-win.exe". Inside the "sand-sim-win" directory, this structure is already setup correctly.

sand-sim can then be run using the "sand-sim-win.exe" executable. 

> [!IMPORTANT]
> Windows Defender may pop up with a [warning message](https://superuser.com/questions/1553842/windows-protected-your-pc) regarding "sand-sim-win.exe" being an unrecognized app. Clicking "More info" and then "Run anyway" will enable the app to run. (sand-sim is not malicious, the well-documented source code can checked to verify this) 

For Linux users, SDL2 can be installed system-wide. 

To do this, on a Debian-based distribution, run the following command:

```bash
sudo apt-get install libsdl2-2.0-0
```

SDL2_image is also required, which can be installed similarly:

```bash
sudo apt-get install libsdl2-image-2.0-0
```

Once SDL2 is installed, sand-sim can be run using the provided binary:

```bash
./sand-sim
```

### Controls

Use the left mouse button the generate tile particles into the world!

The selected element type may be changed using the keyboard or the mouse scroll
wheel. The key controls are:

- 1 - Sand
- 2 - Water
- 3 - Wood
- 4 - Steam
- 5 - Fire

Holding L-CTRL enables changing brush size when scrolling the mouse wheel.

The panel in the topleft represents your currently selected element.

Press ESC to quit the app.


### Command-Line Arguments

The size of the sandbox simulation can be customized at the command-line with
a simple command-line interface:

```bash
Usage: ./sand-sim [options]
Options:
  -h/--help      This message.
  --size         Size preset of sandbox, either "small", "medium", or "large".
  --width        Set tile width of the sandbox. Overrides --size. If specified, height must be specified too.
  --height       Set tile height of the sandbox. Overrides --size. If specified, width must be specified too.
```

Custom widths can range anywhere from 40 to 200. 
Custom heights can range anywhere from 23 to 100.

Non-negative, non-zero custom dimensions which exceed these bounds are bound to
these limits.

For example, on Linux, to set the sandbox to be a size of 50 x 80 would look like the following:

```bash
./sand-sim --width 50 --height 80
```

Similarly on Windows:

```bash
.\sand-sim-win.exe --width 50 --height 80
```

As another example, to use the "large" setting sandbox on Linux:

```bash
./sand-sim --size large
```

And on Windows:
```bash
.\sand-sim-win.exe --size large
```

If no command-line arguments are passed, sand-sim defaults to a 'medium' size
sandbox.

If any command-line arguments are invalid for any reason, sand-sim will repeat
its usage string.

Any custom widths/heights greater than 0 passed to sand-sim through
the CLI will be bound by the width and height of the "small" and "large" size
sandboxes.


## Building From Source

> [!NOTE]
> The compilation instructions below assume a Debian-based Linux enviroment. 

After cloning the project repository, the provided Makefile present at the project root 
can be used to compile sand-sim once the required dependencies are in place.

### Linux

Compiling sand-sim's Linux version requires the development versions of SDL2 and SDL2_image, which can be installed on Debian-based systems with the commands:

```bash
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
```

Compilation of the Linux version requires clang, which can be installed with the command:

```bash
sudo apt-get install clang
```

Once clang is installed, sand-sim can be built from the project root directory using Make and the provided Makefile:

```bash
make
```

This will produce a binary called "sand-sim" which can be executed to run the program.

### Windows

Compiling sand-sim's Windows version requires MingW64 and pkg-config, both of which can be installed with the commands:

```bash
sudo apt-get install mingw-w64
```

```bash
sudo apt-get install pkg-config
```

Once these are installed, sand-sim can be built from the project root directory with the command:

```bash
make sand-sim.exe
```

The necessary SDL2 Windows pre-compiled binaries are provided with the source of
sand-sim and no external installation of SDL2 is necessary for Windows compilation.

As a reminder, in order to run the output Windows binary, "sand-sim.exe" must be placed
next to the correct SDL2 and SDL2_image DLLs. (along with the "assets" directory)

The provided Makefile sets this structure by generating the necessary DLLs next
to "sand-sim.exe" at the project root.

## Project and Source File Organization

- "assets/" - Visual and audio assets used in GUI.
- "libs/" - Pre-compiled third-party library code and headers.
- "src/sandbox.c" - Core sandbox simulation logic.
- "src/gui.c" - Implementation of GUI for displaying sandbox in SDL2.
- "src/utils.c" - General-purpose utility functions.
- Makefile.common - Build definitions common to both Linux and Windows ports.

