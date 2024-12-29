CFLAGS=-Wall -std=c23

graphical.o: graphical.h graphical.c
tetris: tetris.o graphical.o graphical.h pieces.h
