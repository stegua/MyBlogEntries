/********************************************************************************

Spaghetti Optimization - Gomory Cuts using CPLEX callable library

Stefano Gualandi, stefano.gualandi [at] gmail.com

 MIT License

 Copyright (c) 2017 Stefano Gualandi

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 *******************************************************************************/

/*
 * The example file "example.mat" encode the example 8.10 of the
 * Wolsey's book "Intger Programming", Wiley, 1998.

     maximize    4 x_1 -  5 x_2
     subject to  7 x_1 - 2 x_2 + x_3             = 14
                           x_2       + x_4       = 3
                 2 x_1 - 2 x_2             + x_5 = 3
     x_i >= 0 and integer for i in {1,2}  (x_3, x_4, and x_5 are slacks)
*/

#include <ilcplex/cplex.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define TRUE   0
#define FALSE  1
#define MAX_N_ROWS 10
#define POST_CMD(x) if (x) goto QUIT;

/// From IBM-ILOG examples
static void
free_and_null (char **ptr) {
   if ( *ptr != NULL ) {
      free (*ptr);
      *ptr = NULL;
   }
} /* END free_and_null */

/// My Rows structure to store cutting planes
/// First row is the cost vector (i.e. rhs=0)
typedef struct {
   int     n;
   double  rhs;
   int*    ind;
   double* lhs;
} MyRow;

int isInteger(int n, const double* x) {
   int i = 0;
   for ( i = 0; i < n; ++i )
      if ( fabs(round(x[i])) - x[i] > 1e-05 )
         return TRUE;
   return FALSE;
}

int read_matrix(const char* filename, MyRow* rows) {
   int i = 0, j = 0;
   int n = 0, m = 0;
   FILE *in = NULL;
   errno_t err;

   if ((err = fopen_s(&in, filename, "r")) != 0) {
      fprintf(stdout, "Impossible to open file: %s\n", filename);
      exit(EXIT_FAILURE);
   }

   /// Read the number of variables and constraints
   /// of the first problem
   fscanf_s(in, "%d %d", &n, &m);
   /// Add the cost vector as first row
   rows[0].n = n;
   rows[0].ind = (int*)    malloc (n * sizeof(int));
   rows[0].lhs = (double*) malloc (n * sizeof(double));
   for ( j = 0; j < n; ++j ) {
      rows[0].ind[j] = j;
      fscanf_s(in, "%lf", &rows[0].lhs[j] );
   }
   fscanf_s(in, "%lf", &rows[0].rhs );
   /// Read the matrix row A_i and rhs b_i
   for ( i = 1; i < m; ++i ) {
      rows[i].n   = n;
      rows[i].ind = (int*)    malloc (n * sizeof(int));
      rows[i].lhs = (double*) malloc (n * sizeof(double));
      for ( j = 0; j < n; ++j ) {
         rows[i].ind[j] = j;
         fscanf_s(in, "%lf", &rows[i].lhs[j] );
      }
      fscanf_s(in, "%lf", &rows[i].rhs );
   }
   fclose(in);

   return m;
}

void print_solution(int cur_numcols, const double* x, const int* cstat) {
   int j = 0;
   char* basismsg = NULL;

   for (j = 0; j < cur_numcols; j++) {
      printf ( "x%d = %.2lf", j+1, x[j]);
      if ( cstat != NULL ) {
         switch (cstat[j]) {
         case CPX_AT_LOWER:
            basismsg = "Nonbasic at lower bound";
            break;
         case CPX_BASIC:
            basismsg = "Basic";
            break;
         case CPX_AT_UPPER:
            basismsg = "Nonbasic at upper bound";
            break;
         case CPX_FREE_SUPER:
            basismsg = "Superbasic, or free variable at zero";
            break;
         default:
            basismsg = "Bad basis status";
            break;
         }
         printf ("  %s",basismsg);
      }
      printf ("\n");
   }
}

void print_max_lp(int m, const MyRow* rows) {
   int i = 0, j = 0;
   printf("min ");
   for ( j = 0; j < rows[0].n; ++j ) {
      if ( rows[i].lhs[j] >= 0 )
         printf("+");
      printf("%.1f x%d ", rows[0].lhs[j], rows[0].ind[j] + 1);
   }
   printf("\n");
   for ( i = 1; i < m; ++i ) {
      for ( j = 0; j < rows[i].n; ++j ) {
         if ( rows[i].lhs[j] >= 0 )
            printf("+");
         printf("%1.1f x%d ", rows[i].lhs[j], rows[i].ind[j] + 1);
      }
      printf("<= %2.1f\n", rows[i].rhs);
   }
   for ( j = 0; j < rows[0].n; ++j ) {
      printf("x%d >= 0\t", j + 1);
   }
   printf("\n");
}

int cg_solver(int m, MyRow* rows) {
   CPXENVptr     env = NULL;
   CPXLPptr      model = NULL;
   int           status = 0;
   int           error = 0;
   int           i, j;
   int           cur_numrows, cur_numcols;
   int           n_cuts, cut;

   int       solstat;
   double    objval;
   double   *x;
   double   *z;
   int      *cstat;

   int      n0 = rows[0].n;
   int      n1 = rows[0].n+m-1;  /// One slack variable for constraint
   int      h = (m-1)*n0 + m-1;  /// Number of nonzeros

   double*  obj = (double*)malloc(n1*sizeof(double));

   double*   rhs = (double*)malloc((m-1) * sizeof(double));    /// The first row is for the cost vector
   char*    sense = (char*)malloc((m-1) * sizeof(char));

   int*     jnd = (int*)malloc(h * sizeof(int));
   int*     ind = (int*)malloc(h * sizeof(int));
   double*  val = (double*)malloc(h * sizeof(double));

   int      idx = 0;

   int*     rmatbeg;
   int*     rmatind;
   double*  rmatval;
   double*  b_bar;
   char*    gc_sense;
   double*  gc_rhs;

   /// Create environment
   env = CPXopenCPLEX (&status);
   if ( env == NULL ) {
      char  errmsg[CPXMESSAGEBUFSIZE];
      fprintf (stderr, "Could not open CPLEX environment. Status: %d\n", status);
      CPXgeterrorstring (env, status, errmsg);
      fprintf (stderr, "%s", errmsg);
      goto QUIT;
   }

   /// Disable presolve
   POST_CMD( CPXsetintparam (env, CPX_PARAM_PREIND, CPX_OFF) );

   /// Create problem
   model = CPXcreateprob (env, &error, "gomory");
   if (error) goto QUIT;

   /// Minimization problem
   POST_CMD( CPXchgobjsen (env, model, CPX_MIN) );

   /// Add rows (remember first row is cost vector)
   for ( i = 0; i < m-1; ++i ) {
      sense[i]='E';
      rhs[i] = rows[i+1].rhs;
   }
   POST_CMD( CPXnewrows(env, model, m-1, rhs, sense, NULL, NULL) );

   /// Add problem variables
   for ( j = 0; j < n0; ++j )
      obj[j] = rows[0].lhs[j];
   /// Add slack variables
   for ( j = n0; j < n1; ++j )
      obj[j] = 0;
   POST_CMD( CPXnewcols(env, model, n1, obj, NULL, NULL, NULL, NULL) );

   /// Write the full matrix A into the LP (WARNING: should use only nonzeros entries)
   for ( i = 1; i < m; ++i ) {
      for ( j = 0; j < n0; ++j ) {
         jnd[idx] = i-1;
         ind[idx] = rows[i].ind[j];
         val[idx] = rows[i].lhs[j];
         idx++;
      }
      /// Add a slack variable per constraint
      jnd[idx] = i-1;
      ind[idx] = n0+i-1;
      val[idx] = 1.0;
      idx++;
   }
   POST_CMD( CPXchgcoeflist(env, model, idx, jnd, ind, val) );

   /// Optimize the problem
   POST_CMD( CPXlpopt(env, model) );

   /// Check the results
   cur_numrows = CPXgetnumrows (env, model);
   cur_numcols = CPXgetnumcols (env, model);

   x =  (double *) malloc (cur_numcols * sizeof(double));
   z =  (double *) malloc (cur_numcols * sizeof(double));
   cstat = (int *) malloc (cur_numcols * sizeof(int));

   b_bar = (double *) malloc (cur_numrows * sizeof(double));

   POST_CMD( CPXsolution (env, model, &solstat, &objval, x, NULL, NULL, NULL) );
   if ( solstat != 1 ) {
      printf("The solver did not find an optimal solution\nSolver status code: %d\n",solstat);
      exit(0);
   }

   /// Write the output to the screen
   printf ("\nSolution status = %d\t\t", solstat);
   printf ("Solution value  = %f\n\n", objval);

   /// If the solution is integer, is the optimum -> exit the loop
   if ( isInteger(cur_numcols, x) ) {
      fprintf(stdout,"The solution is already integer!\n");
      goto QUIT;
   }

   /// Dump the problem model to 'gomory.lp' for debbuging
   POST_CMD( CPXwriteprob(env, model, "gomory.lp", NULL) );

   /// Get the base statuses
   POST_CMD( CPXgetbase(env, model, cstat, NULL) );

   print_solution(cur_numcols, x, cstat);

   printf("\nOptimal base inverted matrix:\n");
   for ( i = 0; i < cur_numrows; ++i ) {
      b_bar[i] = 0;
      POST_CMD( CPXbinvrow(env, model, i, z) );
      for ( j = 0; j < cur_numrows; ++j ) {
         printf("%.1f ", z[j]);
         b_bar[i] += z[j]*rhs[j];
      }
      printf("\n");
   }

   printf("\nOptimal solution (non basic variables are equal to zero):\n");
   idx = 0;     /// Compute the nonzeros
   n_cuts = 0;  /// Number of fractional variables (cuts to be generated)
   for ( i = 0; i < m-1; ++i ) {
      POST_CMD( CPXbinvarow(env, model, i, z) );
      for ( j = 0; j < n1; ++j ) {
         if ( z[j] >= 0 )
            printf("+");
         printf("%.1f x%d ", z[j], j+1);
         if ( floor(z[j]+0.5) != 0 )
            idx++;
      }
      printf("= %.1f\n", b_bar[i]);
      /// Count the number of cuts to be generated
      if ( floor(b_bar[i]) != b_bar[i] )
         n_cuts++;
   }

   /// Allocate memory for the new data structure
   gc_sense = (char*)   malloc ( n_cuts * sizeof(char) );
   gc_rhs   = (double*) malloc ( n_cuts * sizeof(double) );
   rmatbeg  = (int*)    malloc ( n_cuts * sizeof(int) );
   rmatind  = (int*)    malloc (    idx * sizeof(int) );
   rmatval  = (double*) malloc (    idx * sizeof(double) );

   printf("\nGenerate Gomory cuts:\n");
   idx = 0;
   cut = 0;  /// Index of cut to be added
   for ( i = 0; i < m-1; ++i )
      if ( floor(b_bar[i]) != b_bar[i] ) {
         printf("Row %d gives cut ->   ", i+1);
         POST_CMD( CPXbinvarow(env, model, i, z) );
         rmatbeg[cut] = idx;
         for ( j = 0; j < n1; ++j ) {
            z[j] = floor(z[j]); /// DANGER!
            if ( z[j] != 0 ) {
               rmatind[idx] = j;
               rmatval[idx] = z[j];
               idx++;
            }
            /// Print the cut
            if ( z[j] >= 0 )
               printf("+");
            printf("%.1f x%d ", z[j], j+1);
         }
         gc_rhs[cut] = floor(b_bar[i]); /// DANGER!
         gc_sense[cut] = 'L';
         printf("<= %.1f\n", gc_rhs[cut]);
         cut++;
      }

   /// Add the new cuts
   POST_CMD( CPXaddrows (env, model, 0,
                         n_cuts, idx, gc_rhs, gc_sense,
                         rmatbeg, rmatind, rmatval,
                         NULL, NULL) );

   /// Solve the new LP
   POST_CMD( CPXlpopt(env, model) );

   /// Check the results
   cur_numrows = CPXgetnumrows (env, model);
   cur_numcols = CPXgetnumcols (env, model);

   POST_CMD( CPXsolution (env, model, &solstat, &objval, x, NULL, NULL, NULL) );

   if ( solstat != 1 ) {
      printf("The solver did not find an optimal solution\nSolver status code: %d\n",solstat);
      exit(0);
   }
   /// Write the output to the screen
   printf ("\nSolution status = %d\n", solstat);
   printf ("Solution value = %f\n\n", objval);

   POST_CMD( CPXgetbase(env, model, cstat, NULL) );

   print_solution(cur_numcols, x, cstat);

   free_and_null ((char **) &x);
   free_and_null ((char **) &z);
   free_and_null ((char **) &cstat);
   free_and_null ((char **) &rmatbeg);
   free_and_null ((char **) &rmatind);
   free_and_null ((char **) &rmatval);

QUIT:
   free_and_null ((char **) &x);
   free_and_null ((char **) &z);
   free_and_null ((char **) &cstat);

   free(rhs);
   free(sense);

   free(jnd);
   free(ind);
   free(val);

   if ( error ) {
      char  errmsg[CPXMESSAGEBUFSIZE];
      CPXgeterrorstring (env, error, errmsg);
      fprintf (stderr, "%s", errmsg);
   }

   /* Free up the problem as allocated by CPXcreateprob, if necessary */
   if ( model != NULL ) {
      status = CPXfreeprob (env, &model);
      if ( status ) {
         fprintf (stderr, "CPXfreeprob failed, error code %d.\n", status);
      }
   }

   /* Free up the CPLEX environment, if necessary */
   if ( env != NULL ) {
      status = CPXcloseCPLEX (&env);

      if ( error ) {
         char  errmsg[CPXMESSAGEBUFSIZE];
         fprintf (stderr, "Could not close CPLEX environment.\n");
         CPXgeterrorstring (env, status, errmsg);
         fprintf (stderr, "%s", errmsg);
      }
   }

   return (status);
}

int main(int argc, char* argv[]) {
   MyRow rows[MAX_N_ROWS];
   int i = 0;

   if (argc != 2) {
#ifdef _WIN32
      fprintf(stdout, "\nUsage:\n GomoryCut.exe example.mat\n\n");
#else
      fprintf(stdout, "\nUsage:\n cg_solver example.mat\n\n");
#endif
      exit(EXIT_FAILURE);
   }

   int m = read_matrix(argv[1], rows);
   print_max_lp(m, rows);
   cg_solver(m, rows);

   for ( i = 0; i < m; ++i ) {
      free(rows[i].lhs);
      free(rows[i].ind);
   }

   return 0;
}