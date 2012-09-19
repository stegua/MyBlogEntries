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

#include <boost/graph/graph_traits.hpp> 
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/dijkstra_shortest_paths.hpp>
using namespace boost; 

typedef adjacency_list < vecS, vecS, directedS, no_property, property<edge_weight_t, cost_t> >   Digraph; 
typedef graph_traits<Digraph>::vertex_descriptor  Node; 
typedef graph_traits<Digraph>::edge_descriptor    Arc; 

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
   fprintf(stdout,"n %d, m %d\n", n, m);
   /// Build the graph
   Digraph G (n);
   
   int v, w;
   cost_t c;
   for ( int i = 0; i < m; i++ ) {
      infile >> v >> w >> c;
      add_edge(v-1, w-1, c, G);
   }
   
   vector<Node>    P(n);
   vector<cost_t>  D(n,std::numeric_limits<cost_t>::max());    
   cost_t T_dist;

   timer TIMER;
   for ( int i = 0; i < 50; ++i ) {
      double t0 = TIMER.elapsed();
      node_t S = i;
      node_t T = n-1-i;
      dijkstra_shortest_paths(G, S, predecessor_map(&P[0]).distance_map(&D[0]));
      T_dist = D[T];
      fprintf(stdout,"Time %.4f Cost %"PRId64"\n", TIMER.elapsed()-t0, T_dist);
   }
   fprintf(stdout,"Tot %.4f\n", TIMER.elapsed());

   return T_dist;
}

///------------------------------------------------------------------------------------------
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
   fprintf(stdout,"Cost %" PRId64 " - Time %.3f\n", T_dist, TIMER.elapsed());
   
   return 0;
}
