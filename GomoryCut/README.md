## Introduction
This directory contains a simple program (`cpx_gomory.c`) that takes as input a **small** linear integr program, solves the Linear Programming relaxation using CPLEX, and generates Gomory cuts from the optimal tableu computed by CPLEX.

Given a problem with _n_ variables and _m_ constraints, the problem is:

	min { cx | Ax <= b, x >= 0, x integer }

The input file format is as follows:

	<n> <m + 1>
	<c_1> ... <c_n> 0
	<a_11> ... <a_1n> <b_1>
	...
	<a_m1> ... <a_mn> <m_1>

Since the program is just written for teaching purpose, it supports at most 10 constraints.

### Example
The file `example.mat` encodes Exercise 8.10 in Wolsey's book *Integer Programming*, Wiley, 1998.

To solve the problem, first you need compile the program, and then execute it as:

	% ./cpx_gomory example.mat

The output should be as follows:

	min -4.0 x1 +5.0 x2 
	s.t. +7.0 x1 -1.0 x2 <= 14.0
	     +0.0 x1 +1.0 x2 <= 3.0
	     +2.0 x1 -2.0 x2 <= 3.0
	     x1 >= 0  x2 >= 0  

	Solution status = 1     Solution value  = -6.000000

	x1 = 1.50  Basic
	x2 = 0.00  Nonbasic at lower bound
	x3 = 3.50  Basic
	x4 = 3.00  Basic
	x5 = 0.00  Nonbasic at lower bound

	Optimal base inverted matrix:
	0.0 0.0 0.5 
	0.0 1.0 0.0 
	1.0 0.0 -3.5 

	Optimal solution (non basic variables are equal to zero):
	+1.0 x1 -1.0 x2 +0.0 x3 +0.0 x4 +0.5 x5 = 1.5
	+0.0 x1 +1.0 x2 +0.0 x3 +1.0 x4 +0.0 x5 = 3.0
	+0.0 x1 +6.0 x2 +1.0 x3 +0.0 x4 -3.5 x5 = 3.5

	Generate Gomory cuts:
	Row 1 gives cut ->   +1.0 x1 -1.0 x2 +0.0 x3 +0.0 x4 +0.0 x5 <= 1.0
	Row 3 gives cut ->   +0.0 x1 +6.0 x2 +1.0 x3 +0.0 x4 -4.0 x5 <= 3.0

	Solution status = 1
	Solution value = -4.000000

	x1 = 1.00  Basic
	x2 = 0.00  Nonbasic at lower bound
	x3 = 7.00  Basic
	x4 = 3.00  Basic
	x5 = 1.00  Basic
	
## Requirements
You need to have [CPLEX](http://www-01.ibm.com/software/integration/optimization/cplex-optimizer/) installed on your computer.
