# Stefano Gualandi, stefano.gualandi@gmail.com
# last update, May, 2013

OPTFLAG = -O2 -DDEBUG -Wall -g -fPIC
LDFLAGS = -O2 -DDEBUG -lm -lpthread -g

COMPILER    = clang++ ${OPTFLAG}
LINKER      = clang++ ${LDFLAGS}

# Directory for my files
MYHOME          = ${PWD}
BIN             = ${MYHOME}
INCLUDE         = ${MYHOME}
LIB             = ${MYHOME}
SRC             = ${MYHOME}

# Gecode directory
GECODE_INCLUDE  = /Users/gualandi/LIBS/gecode/include
GECODE_LIB      = -L/Users/gualandi/LIBS/gecode/lib \
   -lgecodedriver -lgecodeint -lgecodekernel -lgecodeminimodel \
   -lgecodesearch -lgecodeset -lgecodesupport 

# Cliquer
CLIQUER_INC = /Users/gualandi/MyLIB/cliquer-1.21/
CLIQUER_LIB = /Users/gualandi/MyLIB/cliquer-1.21/cliquer.o \
   /Users/gualandi/MyLIB/cliquer-1.21/graph.o \
   /Users/gualandi/MyLIB/cliquer-1.21/reorder.o

# My Files
GeCol: ${SRC}/GeCol.cc
	${COMPILER} -c ${SRC}/GeCol.cc -o ${LIB}/GeCol.o -I${GECODE_INCLUDE} -I${INCLUDE} -I${CLIQUER_INC}
	${LINKER} -o ${BIN}/GeCol ${LIB}/GeCol.o ${GECODE_LIB} ${CLIQUER_LIB}

