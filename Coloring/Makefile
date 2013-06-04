# Stefano Gualandi, stefano.gualandi@gmail.com
# last update, May, 2013

OPTFLAG = -O2 -DNDEBUG -Wall -ggdb -fPIC
LDFLAGS = -O2 -DNDEBUG -lm -lpthread -ggdb

COMPILER    = g++ ${OPTFLAG}
LINKER      = g++ ${LDFLAGS}

# Directory for my files
MYHOME          = ${PWD}
BIN             = ${MYHOME}
INCLUDE         = ${MYHOME}
LIB             = ${MYHOME}
SRC             = ${MYHOME}

# Gecode directory
GECODE_INCLUDE  = /data/gualandi/LIBS/gecode-trunk
GECODE_LIB      = -L/data/gualandi/LIBS/gecode-trunk/ \
   -lgecodedriver -lgecodeint -lgecodekernel -lgecodeminimodel \
   -lgecodesearch -lgecodeset -lgecodesupport -lgecodefloat

# Cliquer
CLIQUER_INC = /data/gualandi/LIBS/cliquer-1.21
CLIQUER_LIB = ${CLIQUER_INC}/cliquer.o ${CLIQUER_INC}/graph.o ${CLIQUER_INC}/reorder.o

# My Files
GeColImpact: ${SRC}/GeColImpact.cc
	${COMPILER} -c ${SRC}/GeColImpact.cc -o ${LIB}/GeColImpact.o -I${GECODE_INCLUDE} -I${INCLUDE} -I${CLIQUER_INC}
	${LINKER} -o ${BIN}/GeColImpact ${LIB}/GeColImpact.o ${GECODE_LIB} ${CLIQUER_LIB}

GeCol: ${SRC}/GeCol.cc
	${COMPILER} -c ${SRC}/GeCol.cc -o ${LIB}/GeCol.o -I${GECODE_INCLUDE} -I${INCLUDE} -I${CLIQUER_INC}
	${LINKER} -o ${BIN}/GeCol ${LIB}/GeCol.o ${GECODE_LIB} ${CLIQUER_LIB}

## DSATUR by M.Trick
dsatur: ${SRC}/trick.c
	gcc -c ${SRC}/trick.c -funroll-loops -o ${LIB}/dsatur.o -I${CLIQUER_INC}
	gcc -o ${BIN}/dsatur ${LIB}/dsatur.o ${CLIQUER_LIB}
