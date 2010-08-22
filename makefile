MAIN=introduccion

SRC=introduccion.cpp
CXX=g++
CFLAGS= -g -Wall -O2

OPCIONES=${CFLAGS} -I/usr/include/GL -I/bin
BIBLIOTECAS=-L/lib -lm -L/usr/lib/ -lGL -lglut -lGLU

all: 
	${CXX} ${OPCIONES} ${SRC} -o ${MAIN} ${BIBLIOTECAS}

clean:
	rm -rf ${MAIN}
