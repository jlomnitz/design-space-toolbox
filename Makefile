# Makefile for the Design Space Toolbox V2 C library
# Created for compilation on Unix-like environments.
# The dependencies for the Design Space Toolbox V2 
# are the following:
#      (1)  The GNU Linear Programming Kit (modified for re-enterability).
#      (2)  The GNU Scientific Library
#      (3)  An implementation of cblas, preferable compiled using atlas.
#
# THIS TAG INDICATES THAT THIS MAKEFILE IS NON-FUNCTIONAL.
CC=gcc
CFLAGS= -std=gnu99 -wall -O3 -I/usr/local/include/ -shared -
LIBS = -lglpk -lgsl -lcblas
EXECUTABLE=libdesignspace

SOURCE = $(wildcard *.c)

all: $(SOURCE)
        gcc -o $@ $^ $CFLAGS $LIBS