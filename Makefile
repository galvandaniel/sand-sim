!include Makefile.common

# Makefile as called by NMAKE to compile sand-sim in Windows.
#
# This Makefile is not intended to be called directly, but instead used by
# the auxiliary build script, build.bat, in order for clang to be visible.
# To compile Linux version: build.bat

# Path to SDL dependencies as used by clang + MSVC.
SDL_PATH_VC = libs\VC\SDL2-2.28.5
SDL_IM_PATH_VC = libs\VC\SDL2_image-2.8.8

# SDL2 VC implementation depends on Shell32.dll, Windows shell GUI library.
LDFLAGS_VC = -lShell32

# Manually add SDL2 linker flags due to absence of sdl2-config on Windows.
# Must invoke MSVC flag /subsystem:windows to enable replacement of main (and thus SDL_main) by WinMain for GUI apps.
SDL_CFLAGS_VC = -I$(SDL_PATH_VC)\include -Dmain=SDL_main -L$(SDL_PATH_VC)\lib\x64 -lSDL2 -lSDL2main -Xlinker /subsystem:windows
SDL_IM_CFLAGS_VC = -D_REENTRANT -I$(SDL_IM_PATH_VC)\include -L$(SDL_IM_PATH_VC)\lib\x64 -lSDL2_image

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) $(SDL_CFLAGS_VC) $(SDL_IM_CFLAGS_VC) $(LDFLAGS_VC) -o $(@)
	copy $(SDL_PATH_VC)\lib\x64\SDL2.dll .
	copy $(SDL_IM_PATH_VC)\lib\x64\SDL2_image.dll .

clean:
	del /f $(RM_TARGETS)
