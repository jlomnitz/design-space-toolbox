# Makefile for the Design Space Toolbox V2 C library
# Created for compilation on Unix-like environments.
# The dependencies for the Design Space Toolbox V2 
# are the following:
#      (1)  The GNU Linear Programming Kit (modified for re-enterability).
#      (2)  The GNU Scientific Library
#      (3)  An implementation of cblas, preferable compiled using atlas.
#
# Makefile is intended to be used with the Design Space Toolbox Update Script
# The script can be found at:
#
#       https://bitbucket.org/jglomnitz/toolbox-update-script
#

CC = gcc
# Without this  -L"/usr/lib/atlas-base/" the libcblas.so library is not detected in debian. 
CFLAGS = -std=gnu99 -Wall -O3 -shared -fPIC
DEBUG_CFLAGS = -g -std=gnu99 -Wall -shared -fPIC
LIBS = -L/usr/lib/atlas-base -lglpk -L/usr/lib/gsl  -lgsl -lcblas -lprotobuf-c
#LIBS = -lglpk -lgsl -lcblas 

# Remove the built in commands for yacc and lex files
%.c: %.y
%.c: %.l

EXECUTABLE = libdesignspace.so

HEADERS =$(wildcard *.h)
SOURCE = $(wildcard *.c)
SOURCE := $(filter-out lempar.c, $(SOURCE))
SOURCE := $(filter-out serializationtest.c, $(SOURCE))

all: compile

install: compile
	cd build/
	cp ${EXECUTABLE} /usr/local/lib/
	cp -r designspace /usr/local/include/
	cd ../

compile: $(SOURCE)
	mkdir -p build
	cp $(SOURCE) build/
	cp $(HEADERS) build/
	cd build/
	${CC} -o ${EXECUTABLE} ${CFLAGS} ${SOURCE} ${LIBS}
	mkdir -p designspace
	cp *.h designspace/
	cd ../

debug: $(SOURCE)
	${CC} -o ${EXECUTABLE} ${DEBUG_CFLAGS} ${SOURCE} ${LIBS}

clean:
	rm -f *o
	rm -f ${EXECUTABLE}
	rm -f tests/dstest
	rm -rf ./designspace
	rm -rf ./libdesignspace.so

test: debug
	${CC} -o tests/dstest tests/designspacetest.c -Wall -g -I. -L. -ldesignspace ${LIBS} -Wl,-rpath,.
	./tests/dstest

