CFLAGS = -Wall -gdwarf-4
SRCS = sandbox.c gui.c
HDRS = sandbox.h gui.h

CC = clang
WINCC = x86_64-w64-mingw32-gcc

SDL_CFLAGS = `sdl2-config --cflags --libs`

SDL_CFLAGS_WIN = `../include/SDL2-2.28.5/x86_64-w64-mingw32/bin/sdl2-config --cflags --libs`
SDL_IM_CFLAGS_WIN = `pkg-config --cflags --libs ../include/SDL2_image-2.6.3/x86_64-w64-mingw32/lib/pkgconfig/SDL2_image.pc`

.PHONY: clean

sand: $(HDRS) $(SRCS)
	$(CC) $(CFLAGS) -o sand $(SRCS) $(SDL_CFLAGS) -lSDL2_image -lm

sandwin: sandbox.h gui.h sandbox.c gui.c
	$(WINCC) $(CFLAGS) -o sand $(SRCS) $(SDL_CFLAGS_WIN) $(SDL_IM_CFLAGS_WIN) -lm

test: sandbox.h sandbox.c test.c
	$(CC) $(CFLAGS) -o test sandbox.c test.c -lm

clean:
	rm -rf a.out test sand sand.exe
