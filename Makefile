all: planet

planet: planet.c ppm.c
	gcc planet.c ppm.c -Wall -Wextra -o planet -lX11 -lGL -lGLU -lm ./libggfonts.so

clean:
	rm -f planet
	rm -f *.o

