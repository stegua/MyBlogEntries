# Basic Maximum Clique Algorithm

The ```basic-maxclique.c``` file implements a basic enumerative algorithm
to find the maximum clique in a graph (note that the input is the
graph complement). This implementation is used in one of [my post](http://stegua.github.com)

There is a single dependency from the [CBack](http://www.akira.ruc.dk/~keld/research/CBACK/) 
library, that you have to download by your own.

In order to compile the file type (you need to specify **your path** for CBack):

   gcc -o basic-maxclique -O2 basic-maxclique.c CBack-1.0/SRC/CBack.o LIBS/CBack-1.0/SRC

and then, if you execute the program you should get the following output:

```
   $ ./basic-maxclique
   Graph: 
   0 -> 8 7 
   1 -> 6 5 4 
   2 -> 8 4 3 
   3 -> 7 6 5 
   4 -> 8 
   5 -> 7 
   6 -> 7 
   7 -> 
   8 -> 
   w(G)=4.
   Clique:  0  2  5  6
```

Have fun!
