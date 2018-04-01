LDFLAGS=-lm -lrt
CFLAGS=-Wall -pedantic -std=c99 -g -D_XOPEN_SOURCE=500

main: main.o synthesis.o nco.o


