# Stefano Gualandi, stefano.gualandi@gmail.com
# last update, May, 2013

OPTFLAG = -O2 -DDEBUG -Wall -ggdb -fPIC
LDFLAGS = -O2 -DDEBUG -lm -lpthread -ggdb

COMPILER    = g++ ${OPTFLAG}
LINKER      = g++ ${LDFLAGS}

# Directory for my files
MYHOME          = ${PWD}
BIN             = ${MYHOME}
INCLUDE         = ${MYHOME}
LIB             = ${MYHOME}
SRC             = ${MYHOME}

# Gecode directory
GECODE_INCLUDE  = /Users/gualandi/MyLIB/gecode-trunk
GECODE_LIB      = -L/Users/gualandi/MyLIB/gecode-trunk/ \
   -lgecodedriver -lgecodeint -lgecodekernel -lgecodeminimodel \
   -lgecodesearch -lgecodeset -lgecodesupport -lgecodefloat

# Cliquer
CLIQUER_INC = /Users/gualandi/MyLIB/cliquer-1.21
CLIQUER_LIB = ${CLIQUER_INC}/cliquer.o ${CLIQUER_INC}/graph.o ${CLIQUER_INC}/reorder.o

# My Files
GeCol: ${SRC}/GeCol.cc
	${COMPILER} -c ${SRC}/GeCol.cc -o ${LIB}/GeCol.o -I${GECODE_INCLUDE} -I${INCLUDE} -I${CLIQUER_INC}
	${LINKER} -o ${BIN}/GeCol ${LIB}/GeCol.o ${GECODE_LIB} ${CLIQUER_LIB}

