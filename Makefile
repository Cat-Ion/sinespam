LDFLAGS=-lm -lrt
CFLAGS=-Wall -pedantic -std=c99 -g -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=201112L

main: main.o synthesis.o nco.o parser.o network.o


