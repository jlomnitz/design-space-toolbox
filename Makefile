# Makefile for the Design Space Toolbox V2 C library
# Created for compilation on Unix-like environments.
# The dependencies for the Design Space Toolbox V2 
# are the following:
#      (1)  The GNU Linear Programming Kit (modified for re-enterability).
#      (2)  The GNU Scientific Library
#      (3)  An implementation of cblas, preferable compiled using atlas.
#
CC = gcc-4.6
# Without this  -L"/usr/lib/atlas-base/" the libcblas.so library is not detected in debian. 
CFLAGS = -std=gnu99 -Wall -O3 -shared -fPIC
DEBUG_CFLAGS = -g -std=gnu99 -Wall -shared -fPIC
#LIBS = -L/usr/lib/atlas-base -lglpk -L/usr/lib/gsl -lgsl -lcblas 
LIBS = -lglpk -lgsl -lcblas 
EXECUTABLE = libdesignspace.so 

SOURCE = $(wildcard *.c)
HEADERS = $(wildcard *.h)

all: compile

install: compile
	cp lib/${EXECUTABLE} /usr/local/lib
	cp -r designspace /usr/local/include/

compile: $(SOURCE)
	${CC} -o ${EXECUTABLE} ${CFLAGS} ${LIBS} $^ 
	mkdir -p designspace
	mkdir -p lib
	cp ${HEADERS} designspace/
	mv ${EXECUTABLE} lib/

debug: $(SOURCE)
	${CC} -o ${EXECUTABLE} ${DEBUG_CFLAGS} ${LIBS} $^ 

clean:
	rm -f *o
	rm -f lib/${EXECUTABLE}
	rm -f tests/dstest
	rm designspace/*.h

test: debug
	mkdir -p designspace
	cp *.h designspace/
	${CC} -o tests/dstest tests/designspacetest.c -Wall -g -I. -L. -ldesignspace ${LIBS} -Wl,-rpath,. 
	./tests/dstest

