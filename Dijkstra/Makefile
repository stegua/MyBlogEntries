## Stefano Gualandi, stefano.gualandi at gmail.com
## September, 2012

include config.mac

all: dijkstra dijkstra_bgl dijkstra_lemon dijkstra_or-tools

# Example with (quadratic) super additive objective function
dijkstra: dijkstra.cc
	${COMPILER} -c dijkstra.cc -o dijkstra.o -I${BOOST_INCLUDE}
	${LINKER} -o dijkstra dijkstra.o

dijkstra_bgl: dijkstra_bgl.cc
	${COMPILER} -c dijkstra_bgl.cc -o dijkstra_bgl.o -I${BOOST_INCLUDE}
	${LINKER} -o dijkstra_bgl dijkstra_bgl.o

dijkstra_lemon: dijkstra_lemon.cc
	${COMPILER} -c dijkstra_lemon.cc -o dijkstra_lemon.o -I${LEMON_INCLUDE}
	${LINKER} -o dijkstra_lemon dijkstra_lemon.o ${LEMON_LIB}

dijkstra_or-tools: dijkstra_or-tools.cc
	${COMPILER} -c dijkstra_or-tools.cc -o dijkstra_or-tools.o -I${ORTOOLS_INCLUDE}  \
		-I/Users/stegua/Programming/SRC/or-tools-read-only/dependencies/sources/gflags/src
	${LINKER} -o dijkstra_or-tools dijkstra_or-tools.o -L${ORTOOLS_LIB} -lbase -lgraph -lshortestpaths -lutil \
		-L/Users/stegua/Programming/SRC/or-tools-read-only/dependencies/install/lib -lgflags

clean::
	rm -f *.o
	rm -f *~
