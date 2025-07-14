CC_LINUX = clang
CC_WINDOWS = x86_64-w64-mingw32-gcc
SRCS = $(wildcard src/*.c)

CFLAGS = -Wall -g
LDFLAGS_LINUX = -lSDL2_image -lm
LDFLAGS_WINDOWS = -lm

OUTPUT_LINUX = sand
OUTPUT_WINDOWS = sandwin

# Use SDL2 built-in package config to get SDL2 flags.
SDL_CFLAGS_LINUX = `sdl2-config --cflags --libs`
SDL_CFLAGS_WINDOWS = `include/SDL2-2.28.5/x86_64-w64-mingw32/bin/sdl2-config --cflags --libs`
SDL_IM_CFLAGS_WINDOWS = `pkg-config --cflags --libs include/SDL2_image-2.6.3/x86_64-w64-mingw32/lib/pkgconfig/SDL2_image.pc`

.PHONY: clean

# Linux and Windows compile commands. Default to Linux.
$(OUTPUT_LINUX): $(SRCS)
	$(CC_LINUX) $(CFLAGS) $(SRCS) $(SDL_CFLAGS_LINUX) $(LDFLAGS_LINUX) -o $(@)

$(OUTPUT_WINDOWS): $(SRCS)
	$(CC_WINDOWS) $(CFLAGS) $(SRCS) $(SDL_CFLAGS_WINDOWS) $(SDL_IM_CFLAGS_WINDOWS) $(LDFLAGS_WINDOWS) -o $(@)

clean:
	rm -rf a.out $(OUTPUT_LINUX) $(OUTPUT_WINDOWS).exe
