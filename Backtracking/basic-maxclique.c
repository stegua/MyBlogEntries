#include "CBack.h"

/// Complement of the graph for which
/// we are looking for the maximum clique
/// Graph taken from:
/// * E. Balas and J. Xue. Weighted and Unweighted Maximum Clique
///   Algorithms with Upper Bounds from Fraction Coloring.
///   Algorithmica, vol. 15, pp. 397-412, 1996.
/// 
int n=9;
int E[] = { 
   7, 8,
   4, 5, 6,
   3, 4, 8,
   5, 6, 7,
   8,
   7,
   7
};
int V[10] = { -1, 1, 4, 7, 10, 11, 12, 13, 13, 13};

/// Global data to store the Maximum clique
int LB = 0;
int C[9] = {0};

/// Print best clique
void PrintCount()
{ 
   int i, j;
   printf("Graph: \n");
   for ( i = 0; i < n; i++ ) {
      printf("%d -> ", i);
      for ( j = V[i+1]; j > V[i]; j-- )
         printf("%d ", E[j]);
      printf("\n");
   }
   printf("w(G)=%d.\nClique: \t",LB); 
   for ( i = 0; i < n; i++ )
      if ( C[i] == 1 )
      printf("%d\t", i);
   printf("\n");
}

/// Basic implementation of a max clique algorithm
void MaxClique() {
   int v, w;
   int P = n;  /// Size of the candidate set
   int s = 0;  /// Size of the current clique
   int* S;
   Fiasco = PrintCount;
   S = (int*) Ncalloc(n, sizeof(int));
   for (v = 0; v < n; v++ ) {
      /// If the current clique cannot be extended to a clique
      /// larger than C*, where LB=|C*|, then backtrack
      if ( s + P <= LB )
         Backtrack();
      /// Skip removed vertices
      if ( S[v] < 2 ) {
         /// Choice: Either v is in C (S[v]=1) or is not (S[v]=2)
         S[v] = Choice(2);  
         if ( S[v] == 2 ) {  /// P <- P \ {v} 
            P--;    /// Decrease the size of the candidate set 
         } else {   /// S[v]=1: C <- C u {v}
            s++;    /// Update current clique size
            if ( s > LB ) {
               LB = s;   /// Store the new best clique
               for ( w = 0; w <= v; w++ )
                  C[w] = S[w];
            }
            for ( w = V[v+1]; w > V[v] ; w-- ) 
               if ( S[E[w]] == 0 ) {
                  S[E[w]] = 2;
                  P--; /// Decrease the size of the candidate set
               }
         } 
      }
   }
   Backtrack();
}

main() Backtracking(MaxClique())
