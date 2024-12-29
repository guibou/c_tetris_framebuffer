CFLAGS=-Wall

graphical.o: graphical.h graphical.c
tetris: tetris.o graphical.o graphical.h
