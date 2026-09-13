!include Makefile.common

# Makefile as called by NMAKE to compile sand-sim in Windows.
#
# This Makefile is not intended to be called directly, but instead used by
# the auxiliary build script, build.bat, so clang is available in PATH.
# 
# To compile Windows version: build.bat

# Path to SDL dependencies as used by clang + MSVC.
SDL_PATH_VC = libs\VC\SDL2-2.28.5
SDL_IMAGE_PATH_VC = libs\VC\SDL2_image-2.8.8

# SDL VC implementation depends on Shell32.dll, Windows shell GUI library.
LDFLAGS += -lShell32

# Must manually add flags below due to lack of sdl2-config on Windows.

# SDL compiler/linker flags. 
# Note MSVC flag /subsystem to allow substitution of main by WinMain for GUI apps.
CFLAGS += -I$(SDL_PATH_VC)\include -Dmain=SDL_main
LDFLAGS += -L$(SDL_PATH_VC)\lib\x64 -lSDL2 -lSDL2main -Xlinker /subsystem:windows

# SDL Image compiler/linker flags.
CFLAGS += -I$(SDL_IMAGE_PATH_VC)\include -D_REENTRANT 
LDFLAGS += -L$(SDL_IMAGE_PATH_VC)\lib\x64 -lSDL2_image

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(WIN_OBJS) $(HDRS)
	$(CC) -o $(@) $(WIN_OBJS) $(LDFLAGS)
	copy $(SDL_PATH_VC)\lib\x64\SDL2.dll .
	copy $(SDL_IMAGE_PATH_VC)\lib\x64\SDL2_image.dll .

%.obj: %.c
	$(CC) $(CFLAGS) -o $(@) -c $(<)

clean:
	del /f $(RM_TARGETS)
