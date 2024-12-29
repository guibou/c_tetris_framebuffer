CFLAGS=-Wall -std=c11

graphical.o: graphical.h graphical.c
tetris: tetris.o graphical.o graphical.h pieces.h
