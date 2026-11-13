!include Makefile.common

# Makefile as called by NMAKE to compile sand-sim in Windows.
#
# This Makefile is not intended to be called directly, but instead used by
# the auxiliary build script, build.bat, so clang is available in PATH.
# 
# To compile Windows version: `.\build.bat`

# Path to SDL dependencies as used by clang + MSVC.
SDL_PATH_VC = libs/VC/SDL3-3.4.0
SDL_IMAGE_PATH_VC = libs/VC/SDL3_image-3.4.0

# SDL VC implementation depends on Shell32.dll, Windows shell GUI library.
LDFLAGS = $(LDFLAGS) -lShell32

# Must manually add flags below due to lack of pkg-config on Windows.

# Add SDL compiler/linker flags. 
# Note MSVC flag /subsystem to allow substitution of main by WinMain for GUI apps.
CFLAGS = $(CFLAGS) -I$(SDL_PATH_VC)/include
LDFLAGS = $(LDFLAGS) -L$(SDL_PATH_VC)/lib/x64 -lSDL3 -Xlinker /subsystem:Windows

# Add SDL Image compiler/linker flags.
CFLAGS = $(CFLAGS) -I$(SDL_IMAGE_PATH_VC)/include -D_REENTRANT 
LDFLAGS = $(LDFLAGS) -L$(SDL_IMAGE_PATH_VC)/lib/x64 -lSDL3_image

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(WIN_OBJS) $(HDRS)
	$(CC) -o $(@) $(WIN_OBJS) $(LDFLAGS)
	cp "$(SDL_PATH_VC)/lib/x64/SDL3.dll" .
	cp "$(SDL_IMAGE_PATH_VC)/lib/x64/SDL3_image.dll" .
	cp $(SDL_IMAGE_PATH_VC)/lib/x64/optional/*.dll .

debug: $(WIN_DEBUG_OBJS) $(HDRS)
	$(CC) -o $(OUTPUT).exe $(WIN_DEBUG_OBJS) $(DEBUG_LDFLAGS)
	cp "$(SDL_PATH_VC)/lib/x64/SDL3.dll" .
	cp "$(SDL_IMAGE_PATH_VC)/lib/x64/SDL3_image.dll" .
	cp $(SDL_IMAGE_PATH_VC)/lib/x64/optional/*.dll .

.c.obj:
	$(CC) $(CFLAGS) -o $(@) -c $(<)

.c.debugobj:
	$(CC) $(DEBUG_CFLAGS) -o $(@) -c $(<)

clean:
	rm -f $(RM_TARGETS)
