include Makefile.common

# Makefile as called by GNUmake to compile sand-sim in a Unix-like environment.
#
# To compile the Linux version: make
# To cross-compile the Windows version on Linux: make sand-sim.exe

CC_MINGW = x86_64-w64-mingw32-gcc

# Path to SDL dependencies as used by mingw.
SDL_PATH_MINGW = libs/mingw/SDL2-2.28.5/x86_64-w64-mingw32
SDL_IM_PATH_MINGW = libs/mingw/SDL2_image-2.8.8/x86_64-w64-mingw32

# Use SDL2 built-in package config to get SDL2 linker flags.
SDL_CFLAGS_LINUX = `sdl2-config --cflags --libs`
SDL_CFLAGS_MINGW = `$(SDL_PATH_MINGW)/bin/sdl2-config --cflags --libs`

SDL_IM_CFLAGS_LINUX = -lSDL2_image
SDL_IM_CFLAGS_MINGW = `pkg-config --cflags --libs $(SDL_IM_PATH_MINGW)/lib/pkgconfig/SDL2_image.pc`

.PHONY: clean

# Linux and Windows compile commands. Default to Linux.
$(OUTPUT): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) $(SDL_CFLAGS_LINUX) $(SDL_IM_CFLAGS_LINUX) $(LDFLAGS) -o $(@)

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(SRCS) $(HDRS)
	$(CC_MINGW) $(CFLAGS) $(SRCS) $(SDL_CFLAGS_MINGW) $(SDL_IM_CFLAGS_MINGW) $(LDFLAGS) -o $(@)
	cp $(SDL_PATH_MINGW)/bin/SDL2.dll .
	cp $(SDL_IM_PATH_MINGW)/bin/SDL2_image.dll .

clean:
	rm -rf $(RM_TARGETS)
