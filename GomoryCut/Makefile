## Stefano Gualandi, stefano.gualandi@gmail.com
## February, 2013

OPTFLAG         = -O2 -Wall -fPIC -fexceptions -DNDEBUG 
LDFLAGS 	       = -O2 -Wl,-no_compact_unwind -DNDEBUG -lm -pthread

COMPILER        = llvm-gcc ${OPTFLAG}
LINKER          = llvm-gcc ${LDFLAGS}

# CPLEX directory  (SET YOUR OWN CPLEX DIRECTORIES)
CPLEX_HOME     = /Users/gualandi/Applications/IBM/ILOG/CPLEX_Studio125/cplex
CPLEX_INC      = ${CPLEX_HOME}/include/
CPLEX_LIB      = ${CPLEX_HOME}/lib/x86-64_darwin/static_pic/ -lcplex

# Compile the main file
cpx_gomory: cpx_gomory.c
	${COMPILER} -c cpx_gomory.c -o cpx_gomory.o -I${CPLEX_INC}
	${LINKER} -o cpx_gomory cpx_gomory.o -L${CPLEX_LIB} 

cg_solver: cg_solver.c
	${COMPILER} -c cg_solver.c -o cg_solver.o -I${CPLEX_INC}
	${LINKER} -o cg_solver cg_solver.o -L${CPLEX_LIB} 

gomory: gomory.c
	${COMPILER} -c gomory.c -o gomory.o -I${CPLEX_INC}
	${LINKER} -o gomory gomory.o -L${CPLEX_LIB} 

clean::
	rm -f *.o
	rm -f ${LIB}/*.o
	rm -f *~
	rm -f ${SRC}/*~ ${INCLUDE}/*~
