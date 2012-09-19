/// My typedefs
#include <inttypes.h>
#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>

typedef int32_t    node_t;
typedef int32_t    edge_t;
typedef int64_t    cost_t;

/// From STL library
#include <vector>
using std::vector;

#include <string>

using std::pair;
using std::make_pair;

/// Label for the labeling and/or dijkstra algorithm
enum Label { UNREACHED, LABELED, SCANNED };

/// Data structure to store Key-Value pairs in a 
/// PriorityQueue (as a Fibonacci heap)
struct ValueKey {
   cost_t d;
   node_t u;
   ValueKey(cost_t _d, node_t _u)
      : d(_d), u(_u) {}
   /// The relation establishes the order in the PriorityQueue
   inline bool operator<(ValueKey const & rhs) const { return d < rhs.d; }
};

#include <boost/heap/fibonacci_heap.hpp>
typedef boost::heap::fibonacci_heap<ValueKey>  FibonacciHeap;

#include <boost/heap/d_ary_heap.hpp>
typedef boost::heap::d_ary_heap<ValueKey, boost::heap::arity<2>, boost::heap::mutable_<true> >  BinaryHeap;

#include <boost/heap/d_ary_heap.hpp>
typedef boost::heap::d_ary_heap<ValueKey, boost::heap::arity<3>, boost::heap::mutable_<true> >  TernaryHeap;

#include <boost/heap/skew_heap.hpp>
typedef boost::heap::skew_heap<ValueKey, boost::heap::mutable_<true> >  SkewHeap;

#include <boost/heap/pairing_heap.hpp>
typedef boost::heap::pairing_heap<ValueKey>  PairingHeap;

#include <boost/heap/binomial_heap.hpp>
typedef boost::heap::binomial_heap<ValueKey>  BinomialHeap;

/// Simple Arc class: store tuple (i,j,c)
class Arc {
   public:
      node_t    w;  /// Target node
      cost_t    c;  /// Cost of the arc
      /// Standard constructor
      Arc ( node_t _w, cost_t _c ) 
         : w(_w), c(_c) {}
};

/// Forward and Backward star: intrusive list
typedef std::vector<Arc>                 FSArcList;
typedef FSArcList::iterator              FSArcIter;

///--------------------------------------------------------------------------------
/// Class of graph to compute RCSP with superadditive cost
class Digraph {
   private:
      node_t  n;
      edge_t  m;

      vector<FSArcList>  Nc;   /// Nodes container

      /// Initialize distance vector with Infinity
      /// Maybe it is better to intialize with an upper bound on the optimal path (optimal rcsp path)
      const cost_t Inf;

   public:
      ///Standard constructor
      Digraph( node_t _n, edge_t _m ) 
         : n(_n), m(_m), Inf(std::numeric_limits<cost_t>::max())
      {
         assert( n < Inf && m < Inf );
         Nc.reserve(n);
         /// Reserve memory for the set of arcs
         int avg_degree = m/n+1;
         for ( int i = 0; i < n; ++i ) {
            FSArcList tmp;
            tmp.reserve(avg_degree);
            Nc.push_back(tmp);
         }
      }
      
      void addArc( node_t i, node_t j, cost_t c ) {    
         Nc[i].push_back( Arc(j, c) );
      }
     
      ///--------------------------------------------------
      /// Shortest Path for a graph with positive weights
      /// With a Fibonacci Heap, as given in the book "Algorithms" by Vazirani et all.
      /// NOTE: since 'increase' is O(1), while 'decrease' is O(log n)
      /// We use negative distances in the heap, i.e., we start with distance labels set to -\infinity
      /// However, the distance labels are kept with the correct value 
      template <typename PriorityQueue>
      cost_t spp ( node_t S, node_t T, vector<node_t>& P ) {    
         typedef typename PriorityQueue::handle_type     handle_t;

         PriorityQueue     H;
         vector<handle_t>  K(n);
         vector<Label>     Q(n,UNREACHED);  /// true if it is in the heap
         
         /// Initialize the source distance
         //D[S] = 0;
         K[S] = H.push( ValueKey(0,S) );
         while ( !H.empty() ) {
            /// u = deleteMin(H)
            ValueKey p = H.top();
            H.pop();
            node_t u  = p.u;
            Q[u] = SCANNED;
            cost_t Du = -(*K[u]).d;
            if ( u == T ) { break; }
            /// for all edges (u, v) \in E
            for ( FSArcIter it = Nc[u].begin(), it_end = Nc[u].end(); it != it_end; ++it ) {
               node_t v   = it->w;
               if ( Q[v] != SCANNED ) {
                  cost_t Duv = it->c;
                  cost_t Dv  = Du + Duv;
                  if ( Q[v] == UNREACHED ) {
                     P[v] = u;
                     Q[v] = LABELED;
                     K[v] = H.push( ValueKey(-Dv,v) );
                  } else {
                     if ( -(*K[v]).d > Dv ) {
                        P[v] = u;
                        H.increase( K[v], ValueKey(-Dv,v) );
                     }
                  }
               }
            }
         }
         assert( R[T] == SCANNED );
         return -(*K[T]).d;
      }
};

/// Boost Timer
#include <boost/progress.hpp>
using boost::timer;

#include <fstream>

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
   Digraph G (n, m);
   
   int v, w;
   cost_t c;
   for ( int i = 0; i < m; i++ ) {
      infile >> v >> w >> c;
      G.addArc(v-1, w-1, c);
   }
   
   vector<node_t> P(n);
   cost_t T_dist; 
   
   timer TIMER;
   for ( int i = 0; i < 50; ++i ) {
      double t0 = TIMER.elapsed();
      node_t S = i;
      node_t T = n-1-i;
      T_dist = G.spp<BinaryHeap>(S, T, P);
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
   fprintf(stdout,"Cost %"PRId64" - Time %.3f\n", T_dist, TIMER.elapsed());

   return 0;
}
