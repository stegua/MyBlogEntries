/// My typedefs
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

/// Boost Timer
#include <boost/progress.hpp>
using boost::timer;

/// HashMap by google or-tools
#include "base/logging.h"
#include "base/commandlineflags.h"
#include "base/hash.h"
#include "base/map-util.h"
#include "base/callback.h"
#include "graph/shortestpaths.h"

typedef std::pair<node_t, node_t>                        PairNode;
typedef operations_research::hash_map<PairNode, cost_t>  ArcMap;
typedef operations_research::hash_map<node_t, cost_t>    NodeMap;

/// Graph class as callback as required by Google Or-Tools
struct GraphByCallback {
   vector<NodeMap> C;
   cost_t kDisconnectedDistance;
   GraphByCallback( const vector<NodeMap>& _C, cost_t _kDisconnectedDistance ) 
      : C(_C), kDisconnectedDistance(_kDisconnectedDistance) {}
   inline cost_t hasArc(node_t i, node_t j) {
      NodeMap::iterator p_iter = C[i].find(j);
      if ( p_iter != C[i].end() )
         return p_iter->second;
      else
         return kDisconnectedDistance;
   }
};

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
   int avg_degree = m/n+1;
   vector<NodeMap> A;
   for ( int i = 0; i < n; ++i ) {
      NodeMap node(avg_degree);
      A.push_back(node);
   }
   /// Read arcs from file
   int v, w;
   cost_t c;
   for ( int i = 0; i < m; i++ ) {
      infile >> v >> w >> c;
      A[v-1][w-1] = c;
   }
   
   /// Elaborate input data for Dijkstra's algorithm 
   cost_t kMaxInf = std::numeric_limits<cost_t>::max();
   cost_t T_dist;

   timer TIMER;
   for ( int i = 0; i < 50; ++i ) {
      double t0 = TIMER.elapsed();
      node_t S = i;
      node_t T = n-1-i;
      vector<node_t> paths;
      GraphByCallback call(A,kMaxInf);
      ResultCallback2<cost_t, node_t, node_t>* const arc_callback =
         NewPermanentCallback(&call, &GraphByCallback::hasArc);
      operations_research::DijkstraShortestPath(n, S, T, arc_callback, kMaxInf, &paths);
      /// Compute exact distance (the path is stored in the reversed order)
      T_dist = 0.0;
      for ( unsigned int j = 0; j < paths.size()-1; ++j ) 
         T_dist += A[paths[j+1]][paths[j]];
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
