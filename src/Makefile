.PHONY: clean

test: sandbox.h sandbox.c test.c
	clang -Wall -gdwarf-4 -o test sandbox.c test.c -lm

sand: sandbox.h gui.h sandbox.c gui.c
	clang -Wall -gdwarf-4 -o sand sandbox.c gui.c `sdl2-config --cflags --libs` -lSDL2_image -lm

clean:
	rm -rf a.out test sand
