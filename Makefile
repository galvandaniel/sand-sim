!include Makefile.common

# Makefile as called by NMAKE to compile sand-sim in Windows.
#
# This Makefile is not intended to be called directly, but instead used by
# the auxiliary build script, build.bat, in order for clang to be visible.

# Path to SDL dependencies as used by clang + MSVC.
SDL_PATH_VC = libs\VC\SDL2-2.28.5
SDL_IM_PATH_VC = libs\VC\SDL2_image-2.8.8

# Manually add SDL2 linker flags due to absence of sdl2-config on Windows.
SDL_CFLAGS_VC = -I$(SDL_PATH_VC)\include -Dmain=SDL_main -L$(SDL_PATH_VC)\lib\x86 -lSDL2main -lSDL2 -mwindows
SDL_IM_CFLAGS_VC = -D_REENTRANT -I$(SDL_IM_PATH_VC)\include -L$(SDL_IM_PATH_VC)\lib\x64 -lSDL2_image

# Windows version depends on DLLs which must be copied over to root.
$(OUTPUT).exe: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) $(SDL_CFLAGS_VC) $(SDL_IM_CFLAGS_VC) $(LDFLAGS) -o $(@)
	copy $(SDL_PATH_VC)\lib\x64\SDL2.dll .
	copy $(SDL_IM_PATH_MINGW)\lib\x64\SDL2_image.dll .

clean:
	del /f $(RM_TARGETS)
