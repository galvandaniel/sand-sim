include Makefile.common

# Makefile as called by GNUmake to compile sand-sim in a Debian-based environment.
#
# To compile Linux version: make
# To cross-compile Windows version on Linux: make sand-sim.exe

# If cross-compiling, use MinGW.
WIN_CC = x86_64-w64-mingw32-gcc-win32

# Path to SDL dependencies required by MinGW.
SDL_PATH_MINGW = libs/mingw/SDL3-3.4.2/x86_64-w64-mingw32
SDL_IMAGE_PATH_MINGW = libs/mingw/SDL3_image-3.4.0/x86_64-w64-mingw32

# Linux: Use SDL built-in package config to get compile and linker flags.
$(OUTPUT) debug: CFLAGS += `pkg-config --cflags sdl3 sdl3-image`
$(OUTPUT) debug: LDFLAGS += `pkg-config --libs sdl3 sdl3-image` -lm

# Windows: MinGW has its own variant of pkg-config files for compile and linker flags. 
$(OUTPUT).exe: CFLAGS += `pkg-config --cflags $(SDL_PATH_MINGW)/lib/pkgconfig/sdl3.pc` `pkg-config --cflags $(SDL_IMAGE_PATH_MINGW)/lib/pkgconfig/sdl3-image.pc`
$(OUTPUT).exe: LDFLAGS += `pkg-config --libs $(SDL_PATH_MINGW)/lib/pkgconfig/sdl3.pc` `pkg-config --libs $(SDL_IMAGE_PATH_MINGW)/lib/pkgconfig/sdl3-image.pc`

$(OUTPUT): $(GNU_OBJS) $(HDRS)
	$(CC) -o $(@) $(GNU_OBJS) $(LDFLAGS)
	
debug: $(DEBUG_OBJS) $(HDRS)
	$(CC) -o $(OUTPUT) $(DEBUG_OBJS) $(DEBUG_LDFLAGS)

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(WIN_OBJS) $(HDRS)
	$(WIN_CC) -o $(@) $(WIN_OBJS) $(LDFLAGS)
	cp $(SDL_PATH_MINGW)/bin/SDL3.dll .
	cp $(SDL_IMAGE_PATH_MINGW)/bin/SDL3_image.dll .

# Handle different object types separately.
%.o: %.c %.h
	$(CC) $(CFLAGS) -o $(@) -c $(<)
%.obj: %.c
	$(WIN_CC) $(CFLAGS) -o $(@) -c $(<)
%_debug.o: %.c
	$(CC) $(DEBUG_CFLAGS) -o $(@) -c $(<)

clean:
	rm -rf $(RM_TARGETS)

.PHONY: clean debug
