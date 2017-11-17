.PHONY : clean

raytracer : raytracer-project2.h vector3.c main.c utils.[hc] color.c
	clang -g -Wall -lm -o raytracer utils.c vector3.c color.c main.c intersect.c coord.c ray3.c

clean :
	rm -rf raytracer raytracer.dSYM

