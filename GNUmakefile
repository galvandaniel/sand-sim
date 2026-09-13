include Makefile.common

# Makefile as called by GNUmake to compile sand-sim in a Debian-based environment.
#
# To compile Linux version: make
# To cross-compile Windows version on Linux: make sand-sim.exe

# If cross-compiling, use MinGW.
WIN_CC = x86_64-w64-mingw32-gcc-win32

# Path to SDL dependencies required by MinGW.
SDL_PATH_MINGW = libs/mingw/SDL2-2.28.5/x86_64-w64-mingw32
SDL_IMAGE_PATH_MINGW = libs/mingw/SDL2_image-2.8.8/x86_64-w64-mingw32

# Linux: Use SDL built-in package config to get compile and linker flags.
$(OUTPUT): CFLAGS += `sdl2-config --cflags`
$(OUTPUT): LDFLAGS += `sdl2-config --libs` -lSDL2_image

# Windows: MinGW has its own variant of sdl2-config for compile and linker flags. 
$(OUTPUT).exe: CFLAGS += `$(SDL_PATH_MINGW)/bin/sdl2-config --cflags` `pkg-config --cflags $(SDL_IMAGE_PATH_MINGW)/lib/pkgconfig/SDL2_image.pc`
$(OUTPUT).exe: LDFLAGS += `$(SDL_PATH_MINGW)/bin/sdl2-config --libs` `pkg-config --libs $(SDL_IMAGE_PATH_MINGW)/lib/pkgconfig/SDL2_image.pc`

$(OUTPUT): $(GNU_OBJS) $(HDRS)
	$(CC) -o $(@) $(GNU_OBJS) $(LDFLAGS) 

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(WIN_OBJS) $(HDRS)
	$(WIN_CC) -o $(@) $(WIN_OBJS) $(LDFLAGS)
	cp $(SDL_PATH_MINGW)/bin/SDL2.dll .
	cp $(SDL_IMAGE_PATH_MINGW)/bin/SDL2_image.dll .

# Handle different object types separately.
%.o: %.c
	$(CC) $(CFLAGS) -o $(@) -c $(<)
%.obj: %.c
	$(WIN_CC) $(CFLAGS) -o $(@) -c $(<)

clean:
	rm -rf $(RM_TARGETS)

.PHONY: clean
