# Author: Stefano Gualandi (stefano.gualandi@gmail.com)

# Machines, processes, and resources
set M;
set P;
set R;

# Set of transient resources
set TR;

# Initial given assignment
param x0 {P} symbolic in M;
param x1 {P} symbolic in M;

# Set of services
param ns default 0;
set NS := 1..ns;
set S {NS} within P;

# Set of neighbors
param Nei {M};
param nn := max{m in M} Nei[m];
set NN := 1..nn;
set N {i in NN} := setof {m in M: Nei[m] == i} m;

# Set of locations
param Loc{M};
param nl := max{m in M} Loc[m];
set NL := 1..nl;
set L {i in NL} := setof {m in M: Loc[m] == i} m;

# Resource consumption per process
param Q {P,R};

# Capacity resource-machine matrix
param C  {M,R};
param RC {m in M, r in TR} := C[m,r] - sum {p in P: x0[p] = m} Q[p,r];

# Safety capacity resource-machine matrix
param SC {M,R};

# Spread min per service
param spread{NS};

# Set of balancing ratio (cost)
param nb default 0;
set NB := 1..nb;
param B1{NB} symbolic in R;
param B2{NB} symbolic in R;
param B3{NB} default 0;

# Set of dependencies
set Dep {NS} within NS;
set Ind := setof {s in NS, j in Dep[s]} (j);

# Process Move Costs - PMC_p
param PMC {P};

# Machine Move Costs - MMC_{m1,m2}
param MMC {M,M};

# Weight of the five cost components
param Lw {R}  default 0;
param Bw {NB} default 0;
param Pw default 0;
param Sw default 0;
param Mw default 0;

# Possible assignments
set MP within M cross P default M cross P;

# Fixed processes 
set PF    within P;
set Fixed within MP;

# Mapping of services to ...
set SP {s in NS} := S[s];
set SM {s in NS} within M;
set SN {s in NS} within NN;

set V within NS default {};
set E within NS cross NS default {};
param InDegree {s1 in V} := card({(s2,s1) in E});

# START VARIABLE DEFINITION
var x {MP}     binary;#>= 0, <= 1;
var y {NL, s in NS: spread[s] > 1} >= 0, <= 1;
var z {NN,Ind}  >= 0, <= 1;

# Load costs
var alpha {m in M, r in R}   >= 0, <= (C[m,r] - SC[m,r]);  
# Balance costs
var beta  {NB, M} >= 0; 
# Service move cost
var gamma         >= 0, <= max{s in NS} card(S[s]);  

# Objective function
minimize OverallCost:
	Sw*gamma +
	sum {r in R}  Lw[r]*sum{m in M} alpha[m,r] +
	sum {b in NB} Bw[b]*sum{m in M} beta[b,m] +
	sum {p in P: (x0[p],p) in MP} Pw*PMC[p]*(1 - x[x0[p],p]) +
	Mw*sum {p in P, m2 in M: (m2,p) in MP} MMC[x0[p],m2]*x[m2,p];

# Each process is assigned to a single machine
s.t. Assignment {p in P}:
	sum{(m,p) in MP} x[m,p] = 1;

# Load Cost constraints AND link to load cost variables (0 <= alpha[m,r] <= C[m,r] - SC[m,r]
s.t. LoadCost {m in M, r in R}:
	sum {(m,p) in MP} Q[p,r]*x[m,p] <= alpha[m,r] + SC[m,r];

# Transient Capacity
s.t. CapacityTR {m in M, r in TR}:
	sum {(m,p) in MP: x0[p]<>m} Q[p,r]*x[m,p] <= RC[m,r];

# Balance Cost constraints
s.t. BalanceCost {b in NB, m in M}:
	beta[b,m] >= B3[b]*(C[m,B1[b]] - sum {p in P:(m,p) in MP} Q[p,B1[b]]*x[m,p]) - C[m,B2[b]] + sum {p in P: (m,p) in MP} Q[p,B2[b]]*x[m,p];

# Two processes of the same service cannot stay on the same machine
s.t. Conflict {m in M, s in NS: card(S[s]) > 1}:
	sum {p in S[s]: (m,p) in MP} x[m,p] <= 1;

# Spread constraint
s.t. Spread {s in NS: spread[s] > 1}:
	sum {l in NL} y[l,s] >= spread[s];

s.t. ActiveY {l in NL, s in NS: spread[s] > 1}:
   sum {m in L[l], p in S[s]: (m,p) in MP} x[m,p] >= y[l,s];

# Dependency Constraints
s.t. ActiveZ {n in NN, s in Ind}:
	sum {m in N[n], p in S[s]: (m,p) in MP} x[m,p] >= z[n,s];

s.t. Dependency {n in NN, s in NS, s2 in Dep[s]: card(Dep[s]) >= 1}:
	sum {m in N[n], p in S[s]: (m,p) in MP} x[m,p] <= card(S[s])*z[n,s2];

# Service move cost (16)
s.t. ServiceMoveCost {s in NS}:
	gamma >= card(S[s]) - sum {p in S[s], m in M: x0[p] = m and (m,p) in MP} x[m,p];
