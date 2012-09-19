/// My typedefs
#include <inttypes.h>
#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>

typedef int32_t    node_t;
typedef int32_t    edge_t;
typedef int64_t    cost_t;

/// From STL library
#include <fstream>

#include <vector>
using std::vector;

#include <string>

using std::pair;
using std::make_pair;


/// Lemon Graph Library
#include <lemon/smart_graph.h>
using lemon::SmartDigraph;

#include <lemon/adaptors.h>
#include <lemon/concepts/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/path.h>
typedef SmartDigraph::Arc  Arc;
typedef SmartDigraph::Node Node;
typedef SmartDigraph::ArcMap<cost_t>   LengthMap;

#include <lemon/fib_heap.h>
typedef SmartDigraph::NodeMap<int>  NodeMap;
typedef lemon::FibHeap<cost_t, NodeMap> FibonacciHeap;

/// Boost Timer
#include <boost/progress.hpp>
using boost::timer;

using namespace boost;

/// Read input data, build graph, and run Dijkstra
cost_t runDijkstra( char* argv[] ) {
   /// Read instance from the OR-lib
   std::ifstream infile(argv[1]); 
   if (!infile) 
      exit ( EXIT_FAILURE ); 

   int n;     /// Number of variables
   int m;     /// Number of constraints

   // reads file of the form
   // #nodes #edges
   // e_1 = v_i v_j cost[e_m]
   // ..
   // e_m = v_i v_j cost[e_m]
   
   /// Read the first line
   infile >> n >> m;
   fprintf(stdout,"n %d, m %d\t", n, m);
   /// Build the graph 
   SmartDigraph G;
   G.reserveNode(n);
   G.reserveArc(m);
   vector<Node> vs;
   vs.reserve(n);
   for ( int i = 0; i < n; ++i )
      vs.push_back( G.addNode() );

   int v, w;
   cost_t c;
   cost_t T_dist; 
   LengthMap    C(G);
   for ( int i = 0; i < m; i++ ) {
      infile >> v >> w >> c;
      Arc a;
      a = G.addArc(vs[v-1], vs[w-1]);
      C[a] = c;
   }
   
   timer TIMER;
   for ( int i = 0; i < 50; ++i ) {
      double t0 = TIMER.elapsed();
      Node S = vs[i];
      Node T = vs[n-1-i];
      //NodeMap heap_cross_ref(G);
      //FibonacciHeap heap(heap_cross_ref);
      //lemon::Dijkstra<SmartDigraph, LengthMap>::SetHeap<FibonacciHeap, NodeMap>::Create spp(G, C);
      //spp.heap( heap, heap_cross_ref );  
      lemon::Dijkstra<SmartDigraph, LengthMap> spp(G, C);
      spp.run(S,T);
      T_dist = spp.dist(T);
      fprintf(stdout,"Time %.4f Cost %"PRId64"\n", TIMER.elapsed()-t0, T_dist);
   }
   fprintf(stdout,"Tot %.4f\n", TIMER.elapsed());

   return T_dist;
}

/// Main function
int
main (int argc, char **argv)
{
   if ( argc != 2 ) {
      fprintf(stdout, "usage: ./dijkstra <filename>\n");
      exit ( EXIT_FAILURE );
   }
   /// Measure overall time
   timer TIMER;
   /// Invoke the different Dijkstra algorithm implementations
   cost_t T_dist = runDijkstra(argv);
   /// Print basic figures
   fprintf(stdout,"Cost %"PRId64" - Time %.3f\n", T_dist, TIMER.elapsed());

   return 0;
}
