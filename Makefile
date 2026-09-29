!include Makefile.common

# Makefile as called by NMAKE to compile sand-sim in Windows.
#
# This Makefile is not intended to be called directly, but instead used by
# the auxiliary build script, build.bat, so clang is available in PATH.
# 
# To compile Windows version: `.\build.bat`

# Replace directory delimiter from '/' to '\'. 
RM_TARGETS = $(RM_TARGETS:/=\)

# Path to SDL dependencies as used by clang + MSVC.
SDL_PATH_VC = libs\VC\SDL3-3.4.0
SDL_IMAGE_PATH_VC = libs\VC\SDL3_image-3.4.0

# SDL VC implementation depends on Shell32.dll, Windows shell GUI library.
LDFLAGS = $(LDFLAGS) -lShell32

# Must manually add flags below due to lack of pkg-config on Windows.

# Add SDL compiler/linker flags. 
# Note MSVC flag /subsystem to allow substitution of main by WinMain for GUI apps.
CFLAGS = $(CFLAGS) -I$(SDL_PATH_VC)\include
LDFLAGS = $(LDFLAGS) -L$(SDL_PATH_VC)\lib\x64 -lSDL3 -Xlinker /subsystem:windows

# Add SDL Image compiler/linker flags.
CFLAGS = $(CFLAGS) -I$(SDL_IMAGE_PATH_VC)\include -D_REENTRANT 
LDFLAGS = $(LDFLAGS) -L$(SDL_IMAGE_PATH_VC)\lib\x64 -lSDL3_image

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(WIN_OBJS) $(HDRS)
	$(CC) -o $(@) $(WIN_OBJS) $(LDFLAGS)
	copy $(SDL_PATH_VC)\lib\x64\SDL3.dll .
	copy $(SDL_IMAGE_PATH_VC)\lib\x64\SDL3_image.dll .
	copy $(SDL_IMAGE_PATH_VC)\lib\x64\optional .

.c.obj:
	$(CC) $(CFLAGS) -o $(@) -c $(<)

clean:
	del /f $(RM_TARGETS)
