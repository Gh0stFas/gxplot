CC = gcc
INC = ./inc
LIB = ./lib
BIN = ./bin
OPT = -O2
LIBFLAGS = ${OPT} -mfpmath=sse -msse2 -c -fpic -I${INC} -D_GNU_SOURCE
# Starting with GCC 4.6, I've had to place the -Wl,--no-as-needed option for
# the linker so it can link the math library properly. Without this option you
# would have to put the math library in an order dependent manner....bleh
CFLAGS = ${opt} -mfpmath=sse -msse2 -lX11 -Wl,--no-as-needed -lm -L${LIB} -I${INC} -D_GNU_SOURCE
OBJ = libfileio.a libplot.a
EXECUTABLE = gxplot

all : ${EXECUTABLE}

${EXECUTABLE} : ${OBJ} gxplot.c ${INC}/plot.h ${INC}/eventnames.h 
	${CC} -o gxplot gxplot.c ${CFLAGS} -lfileio -lplot

libplot.a : ${LIB}/plot/plot.c ${INC}/plot.h
	${CC} ${LIBFLAGS} -o ${LIB}/plot/plot.o ${LIB}/plot/plot.c
	ar -cvq ${LIB}/libplot.a ${LIB}/plot/plot.o
	
libfileio.a : bluefile.a detfile.a ${LIB}/fileio/fileio.c ${INC}/fileio.h 
	${CC} ${LIBFLAGS} -o ${LIB}/fileio/fileio.o ${LIB}/fileio/fileio.c
	ar -cvq ${LIB}/libfileio.a ${LIB}/fileio/fileio.o ${LIB}/fileio/bluefile.o ${LIB}/fileio/detfile.o
	
bluefile.a : ${LIB}/fileio/bluefile.c ${INC}/bluefile.h
	${CC} ${LIBFLAGS} -o ${LIB}/fileio/bluefile.o ${LIB}/fileio/bluefile.c
	ar -cvq ${LIB}/libbluefile.a ${LIB}/fileio/bluefile.o
	
detfile.a : ${LIB}/fileio/detfile.c ${INC}/detfile.h
	${CC} ${LIBFLAGS} -o ${LIB}/fileio/detfile.o ${LIB}/fileio/detfile.c
	ar -cvq ${LIB}/libdetfile.a ${LIB}/fileio/detfile.o

clean :
	/bin/rm -f gxplot ${LIB}/*.a ${LIB}/*/*.o ${BIN}/*

install :
	/bin/mkdir -p ${BIN}
	/bin/mv gxplot ${BIN}
