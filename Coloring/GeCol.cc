/*
 *  Main authors:
 *     Stefano Gualandi <stefano.gualandi@gmail.com>
 *
 *  Copyright:
 *     Stefano Gualandi, 2010
 *
 *  Last update: May, 4th, 2013
 */

/// Limits on numbers
#include <limits>
using std::numeric_limits;
#include <vector>
using std::vector;

#include <gecode/driver.hh>


/// Use Cliquer to find all maximal clique in the conflict graph
extern "C" {
#include "cliquer.h"
#include "set.h"
#include "graph.h"
#include "reorder.h"
}

using namespace Gecode;
using namespace Gecode::Int;

int MYMETHOD = 0;
float SCALE = 17;
set_t    C = NULL;


/// Cutoff object for the script
class MyCutoff : public Search::Stop {
   private:
      Search::TimeStop*    ts; 
      Search::MemoryStop*  ms; 

      MyCutoff(int time, int memory)
         : ts((time > 0)   ? new Search::TimeStop(time) : NULL),
         ms((memory > 0) ? new Search::MemoryStop(memory) : NULL) {}
   public:
      virtual bool stop(const Search::Statistics& s, const Search::Options& o) {
         return
            ((ts != NULL) && ts->stop(s,o)) ||
            ((ms != NULL) && ms->stop(s,o));
      }
      virtual bool stopTime(const Search::Statistics& s, const Search::Options& o) {
         return
            ((ts != NULL) && ts->stop(s,o));
      }
      virtual bool stopMemory(const Search::Statistics& s, const Search::Options& o) {
         return
            ((ms != NULL) && ms->stop(s,o));
      }
      static Search::Stop*
         create(int time, int memory) {
            if ((time == 0) && (memory == 0))
               return NULL;
            else
               return new MyCutoff(time,memory);
         }
      ~MyCutoff(void) {
         delete ts; delete ms;
      }
};


static double trampoline(const Space& home, IntVar x, int i); 
double mymerit(const Space& home, IntVar x, int i) { 
   return x.afc(home) / static_cast<double>(x.size());
}

/// Main Script
class GraphColoring : public Script {
   protected:
      /// Mapping vertices to colors
      IntVarArray   x;  
   public:
      /// Actual model
      GraphColoring( const graph_t*  g, int k, 
            const set_t* CC, int n_c, const set_t* Cs, const vector<int>& init ) 
         :  x ( *this, g->n, 0, k-1 )     /// Colors start from '1'                  
      { 
         int n = g->n;
         if ( !init.empty() ) {
            printf("Init\n");
            for ( int i = 0; i < n; ++i )
               if ( init[i] > 0 ) 
                  rel ( *this, x[i], IRT_EQ, init[i]);
            for ( int i = 0; i < n-1; ++i ) 
               for ( int j = i+1; j < n ; ++j )
                  if ( GRAPH_IS_EDGE(g,i,j) )
                     rel ( *this, x[i], IRT_NQ, x[j]);
         }

         /// Post an 'alldifferent' constraints for each maximal cliques
         for ( int i = 0; i < n_c; ++i )
            if ( set_size(Cs[i]) > 0 ) {
               IntVarArgs xdiff( set_size(Cs[i]) );
               int idx = 0;
               for ( int j = 0; j < n; ++j ) 
                  if ( SET_CONTAINS_FAST(Cs[i],j) ) {
                     xdiff[idx] = x[j];
                     idx++;
                  }

               distinct ( *this, xdiff, ICL_DOM );       
            } 
   
         /*for ( int i = 0; i < n-1; ++i ) 
            for ( int j = i+1; j < n ; ++j )
               if ( GRAPH_IS_EDGE(g,i,j) || GRAPH_IS_EDGE(g,j,i) )
                  rel ( *this, x[i], IRT_NQ, x[j]);*/
         /// Symmetry breaking
         Symmetries syms;
         syms << ValueSymmetry(IntArgs::create(k,0));
         Rnd r(13U*k);
         if ( MYMETHOD == 0 ) 
            branch(*this, x, tiebreak(INT_VAR_SIZE_MIN(), INT_VAR_RND(r)), INT_VAL_MIN());
         if ( MYMETHOD == 1 ) 
            branch(*this, x, tiebreak(INT_VAR_SIZE_MIN(), INT_VAR_RND(r)), INT_VAL_MIN(), syms);
         if ( MYMETHOD == 2 ) 
            branch(*this, x, tiebreak(INT_VAR_AFC_MAX(), INT_VAR_RND(r)), INT_VAL_MIN(), syms);
         if ( MYMETHOD == 3 ) 
            branch(*this, x, tiebreak(INT_VAR_ACTIVITY_MAX(), INT_VAR_RND(r)), INT_VAL_MIN(), syms);
         if ( MYMETHOD == 4 ) 
            branch(*this, x, tiebreak(INT_VAR_ACTIVITY_SIZE_MAX(), INT_VAR_RND(r)), INT_VAL_MIN(), syms);
         if ( MYMETHOD == 5 ) 
            branch(*this, x, tiebreak(INT_VAR_AFC_SIZE_MAX(), INT_VAR_RND(r)), INT_VAL_MIN(), syms );
         if ( MYMETHOD == 6 ) 
            branch(*this, x, tiebreak(INT_VAR_DEGREE_SIZE_MAX(), INT_VAR_RND(r)), INT_VAL_MIN(), syms);
         if ( MYMETHOD == 7 ) 
            branch(*this, x, tiebreak(INT_VAR_MERIT_MAX(&trampoline), INT_VAR_RND(r)), INT_VAL_MIN(), syms );
      }

      GraphColoring( bool share, GraphColoring& s) : Script(share,s) {
         x.update ( *this, share, s.x );
      }

      /// Perform copying during cloning
      virtual Space* copy(bool share) {
         return new GraphColoring(share, *this);
      }

      virtual int getChi(void) {
         int k = 0;
         for ( int i = x.size(); i--; )
            k = std::max( k, x[i].val() );
         return k+1;
      }

      virtual void getColoring(vector<int>& certificate) {
         for ( int i = 0; i < certificate.size(); ++i ) 
            certificate[i] = x[i].val();
      }
      double merit(IntVar x, int i) { 
         if ( SET_CONTAINS(C,i) )
            return std::numeric_limits<double>::max();
         else
            return x.afc(*this) / static_cast<double>(x.size());
      }
};


static double trampoline(const Space& home, IntVar x, int i) { 
   return static_cast<const GraphColoring&>(home).merit(x,i);
}

/// Find upper bounds to coloring
vector<int>
colorFinal ( const graph_t*    g, 
      vector<int>&    initial,
      const set_t*    maxClique,
      int             n_c,        /// Number of cliques in "cliques"
      const set_t*    cliques,
      int             UB,
      double          time,
      int             memory,
      int             n_threads
      )
{
   vector<int> certificate(g->n, 0);
   /// Solution of the problem
   Support::Timer t;
   t.start();

   Search::Options so;
   so.threads = n_threads;
   so.clone   = false;

   GraphColoring* s = new GraphColoring ( g, UB, maxClique, n_c, cliques, initial );
   DFS<GraphColoring> e(s, so);
   GraphColoring* ex = e.next();
   if ( ex == NULL ) {
      printf("Puta!\n");
      exit(EXIT_FAILURE);
   }
   ex->getColoring(certificate);
   delete ex;
   
   fprintf(stdout, "FixTim %.2f", t.stop()/1000);

   return certificate;
}

/// Find upper bounds to coloring
vector<int>
colorHeuristic ( const graph_t*    g, 
      const set_t*    maxClique,
      int             n_c,        /// Number of cliques in "cliques"
      const set_t*    cliques,
      int             UB,
      double          time,
      int             memory,
      int             n_threads
      )
{
   /// For storing the coloring certificate
   vector<int> certificate(g->n, 0);
   vector<int> empty;
   /// Solution of the problem
   Support::Timer t;
   t.start();
   /// Aux variables
   double elapsed;
   double ss = SCALE/10;
   int scriptMemory = 0;
   int tot_nodes = 0;
   int status = 1;
   /// Search loop
   do {
      UB--; /// Find a coloring of better cost

      GraphColoring* s = new GraphColoring ( g, UB, maxClique, n_c, cliques, empty );
      elapsed = time - t.stop();

      if ( elapsed <= 1e-04 ) {
         fprintf(stdout,"\ttimeout of CP solver elapsed\n");
         status = 0;
         break;
      }

      /// Search options
      Search::Cutoff* c = Search::Cutoff::geometric(1000,ss);
      Search::Options so;
      so.stop = MyCutoff::create( elapsed, memory );
      so.threads = n_threads;
      so.clone   = false;
      so.cutoff = c;
      RBS<DFS,GraphColoring> e(s, so);

      GraphColoring* ex = e.next();
      
      scriptMemory = std::max<int>(scriptMemory, e.statistics().memory);

      if ( e.stopped() ) {
         fprintf(stdout,"\tWARNING: STOPPED, IT IS ONLY AN UPPER BOUND!\n");

         MyCutoff* myc = dynamic_cast<MyCutoff *>(so.stop);
         if ( myc->stopTime(e.statistics(),so) ) {
            fprintf(stdout," TIME LIMIT!\n");
            status = 0;
         }
         if ( myc->stopMemory(e.statistics(),so) ) {
            fprintf(stdout," MEMORY LIMIT!\n");
            status = 2;
         }
      }

      tot_nodes += e.statistics().node;

      if ( ex == NULL ) {
         printf("ex=NULL\n");
         UB++;
         break;
      }
      UB = std::min(UB,ex->getChi());
      ex->getColoring(certificate);
      fprintf(stdout,"\t%.2f\t%d\t%d\t%ld\n", (t.stop()/1000), UB, scriptMemory, e.statistics().node);

      delete ex;
   } while ( time - t.stop() >= 0.001 );

   double tend = t.stop()/1000;
   if ( tend > time-1e-05 ) {
      tend = time;
      status = 0;
   }

   fprintf(stdout, "Nodes %d Time %.2f status %d X(G) %d \n", tot_nodes, tend, status, UB);

   return certificate;
}

/// MAIN PROGRAM
int main(int argc, char **argv)
{
   if ( argc == 1 ) {
      fprintf(stdout, "\nusage:  $ ./GeCol <filename>\n\n");
      exit(EXIT_SUCCESS);
   }

   if ( argc >= 3 )
      MYMETHOD = atoi(argv[2]);
   else
      MYMETHOD = 0;

   if ( argc == 4 )
      SCALE = atoi(argv[3]);

   int  timeout = 3600*1000;  /// 1000 seconds timeout
   int  memoryLimit = numeric_limits<int>::max();
   int  threads = 4;
   
   clique_options* opts;
   graph_t* g;  
   graph_t* g0;  
   graph_t* h;  
   graph_t* G;  
   set_t    s;
   int      n_c = 0;   /// Number of maximal cliques found
   set_t*   Cs;        /// Collection of maximal cliques
   int*     table;
   int LB = 0;
   int UB;
   int n;
   int m;

   /// Set the options for using Cliquer
   opts = (clique_options*) malloc (sizeof(clique_options));
   opts->time_function=NULL;
   opts->reorder_function=reorder_by_greedy_coloring;
   opts->reorder_map=NULL;
   opts->user_function=NULL;
   opts->user_data=NULL;
   opts->clique_list=NULL;
   opts->clique_list_length=0;


   /// Read a graph instance in any DIMACS format (binary or ascii)
   g0 = graph_read_dimacs_file(argv[1]);
   table = reorder_by_degree(g0,FALSE);

   n = g0->n;
   m = graph_edge_count(g0);
   UB = n;
   s = set_new(n);
   
   /// Reorder the graph
   g = graph_new(n);
   h = graph_new(n);
   G = graph_new(n);
   vector<int> inver(n,0);
   for ( int i = 0; i < n-1; ++i ) 
      for ( int j = i+1; j < n ; ++j )
         if ( GRAPH_IS_EDGE(g0,table[i],table[j]) ) {
            GRAPH_ADD_EDGE(g,i,j);
            GRAPH_ADD_EDGE(h,i,j);
            GRAPH_ADD_EDGE(G,i,j);
            inver[table[i]] = i;
            inver[table[j]] = j;
         }

   float density = (float)n*(n-1)/2;
   /// Allocate space to store maximal cliques
   Cs = (set_t*)malloc(m*sizeof(set_t));
   C  = set_new(n);

   Support::Timer t;
   t.start();
   
   /// Loop until at least a vertex is removed
   int  n_r = 0;
   while ( graph_edge_count(h) > 0 ) {
      bool flag = false;
      /// Find a maximal clique for every vertex, and store the largest
      for ( int i = 0; i < n; ++i ) 
         if ( graph_vertex_degree(h,i) > 0 ) {
            if ( n > 1000 || density > 0.7 )
               s = clique_find_single ( h, 2, 0, TRUE, opts);
            else
               s = clique_find_single ( h, 0, 0, TRUE, opts);
            maximalize_clique(s,G);
            if ( s != NULL ) {
               if ( set_size(s) > LB ) {
                  LB = set_size(s);
                  set_copy(C,s);  /// C is the best maximal clique found
                  flag = true;
               } 
               Cs[n_c++]=set_duplicate(s);
               /// Rimuovi tutti gli archi contenuti nella clique
               for ( int j = 0; j < n; ++j )
                  if ( SET_CONTAINS_FAST(s,j) )
                     for ( int l = j+1; l < n; ++l )
                        if ( SET_CONTAINS_FAST(s,l) && GRAPH_IS_EDGE(h,j,l) )
                           GRAPH_DEL_EDGE(h,j,l);
            }
            break;
         }

      /// Reduce the graph (if degree smaller than LB, then remove the vertex)
      if ( flag )
         for ( int i = 0; i < n-1; ++i ) {
            if ( graph_vertex_degree(g, i) >= 0 && graph_vertex_degree(g, i) < LB ) { 
               for ( int j = i+1; j < n ; ++j )
                  if ( GRAPH_IS_EDGE(g,i,j) ) {
                     GRAPH_DEL_EDGE(g,i,j);
                     if ( GRAPH_IS_EDGE(h,i,j) )
                        GRAPH_DEL_EDGE(h,i,j);
                     n_r++;
                  }
            }
         }
   }
   fprintf(stdout,"Preprocessing: edge removal %d cliques %d\n", n_r, n_c);

   /// Remove duplicate cliques
   int n_d = n_c;
   for ( int i = 0; i < n_c-1 ; i++ ) {
      for ( int j = i+1; j < n_c ; j++ ) {
         if ( set_size(Cs[i]) == set_size(Cs[j]) ) {
            set_t res = set_new(n);
            set_intersection(res, Cs[i], Cs[j]);
            if ( set_size(Cs[i]) == set_size(res) ) {
               Cs[i] = set_new(n);
               n_d--;
            }
         }
      }
   }

   fprintf(stdout, "Edges %d - Useful maximal cliques %d\n", m, n_d);

   /// Stats on cliques
   int mm = 0;
   for ( int i = 0; i < n_c; i++ )
      if ( set_size(Cs[i]) == LB ) {
         int dd = 0;
         for ( int j = 0; j < n; ++j )
            if ( SET_CONTAINS(Cs[i],j) )
               dd += graph_vertex_degree(g,j);
         if ( dd > mm ) {
            mm = dd;
            set_copy(C, Cs[i]);  /// C is the best maximal clique found
         }
      } 

   /// Call the CP model 
   fprintf(stdout, "Run CP model with LB %d - Preproc time %.3f\n", LB, t.stop()/1000);
   vector<int> certificate = colorHeuristic ( G, &C, n_c, Cs, UB, timeout, memoryLimit, threads );
   
   for ( int i = 0; i < n-1; ++i ) 
      for ( int j = i+1; j < n ; ++j )
         if ( GRAPH_IS_EDGE(g,i,j) && certificate[i] == certificate[j] ) {
            printf("ERROR: No feasible coloring!\n");
            exit(-1);
         }
   
   /// Costruct the final coloring
   for ( int i = 0; i < n; ++i )
      if ( graph_vertex_degree(g,i) < LB )
         certificate[i] = 0;

   vector<int> sol = colorFinal ( G, certificate, &C, n_c, Cs, UB, timeout, memoryLimit, threads );

   for ( int i = 0; i < n-1; ++i ) 
      for ( int j = i+1; j < n ; ++j )
         if ( GRAPH_IS_EDGE(g0,i,j) && sol[inver[i]] == sol[inver[j]] ) {
            printf("ERROR: No feasible coloring!\n");
            exit(-1);
         }

   /// Print the coloring certificate
   printf("\ncertificate: ");
   for ( int i = 0; i < n; ++i ) 
      printf("%d ", sol[inver[i]]);
   printf("\n");
   /// Free the memory used by Cliquer
   if ( s != NULL )
      set_free(s);
   if ( C != NULL )
      set_free(C);
   free(Cs);
   free(opts);
   graph_free(g0);
   graph_free(g);
   graph_free(h);
   graph_free(G);

   return EXIT_SUCCESS;
}
