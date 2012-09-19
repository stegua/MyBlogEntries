---
layout: post
title: "Dijkstra, Dantzig, and Shortest Paths"
date: 2012-09-19 22:14
comments: true
categories: [heap, boost, C++, duality, algorithm, data structure, implementation]
published: false
---

<style type="text/css">
table { width:100%; }
thead {
   background-color: rgba(0,0,255,0.3);
color: black;
       text-indent: 14px;
       text-align: left;
}
td { padding:4px; }
tbody tr:nth-child(odd) {
   background-color: rgba(0, 0, 100, 0.2); /* green, 10% alpha */
}
tbody tr:nth-child(even) {
   background-color: rgba(0, 0, 100, 0.1); /* red, 30% alpha */
}
</style>

Here we go, my first blog entry, ever. Let start with two short quizzes.

**1. The well known Dijkstra's algorithm is:**  
[a]  A greedy algorithm  
[b]  A dynamic programming algorithm  
[c]  A primal-dual algorithm  
[d]  It was discovered by Dantzig  

**2. Which is the best C++ implementation of Dijkstra's algorithm among the following?**  
[a] The [Boost Graph Library (BGL)](http://www.boost.org/doc/libs/1_51_0/libs/graph/doc/index.html)  
[b] The COIN-OR Lemon Graph Library [COIN-OR Lemon](http://lemon.cs.elte.hu/trac/lemon)  
[c] The [Google OrTools](http://code.google.com/p/or-tools/)  
[d] Hei dude! We can do better!!!  

What is your answer for the first question? ... well, the answers are all correct!
And for the second question? To know the correct answer, sorry, you have to read this post to the end...

If you are curious to learn more about the _classification_ of the Dijkstra's algorithm proposed in the first three answers,
please consider reading [1] and [2]. Honestly, I did not know that the algorithm was independently discovered by Dantzig [3] as a special
case of Linear Programming. 
However, Dantzig is credited for the first version of the bidirectional 
Dijkstra's algorithm (should we called it **Dantzig's algorithm**?), which is nowadays the most performing
algorithm on general graphs.
The bidirectional Dijkstra's algorithm is used as benchmark to measure the speed-up of modern specialized
shortest path algorithms for road networks [4,5], those algorithms that are implemented, for instance, in our
GPS navigation systems, in yours smartphones (I don't have one, argh!), in Google Maps Directions, and Microsoft Bing Maps. 

Why a first blog entry on Dijkstra's algorithm? That's simple.

* Have you ever implemented an _efficient_ version of this well-known and widely studied algorithm?
* Have you ever used the version that is implemented in well-reputed graph libraries, such as, the 
[Boost Graph Library (BGL)](http://www.boost.org/doc/libs/1_51_0/libs/graph/doc/index.html), 
the [COIN-OR Lemon](http://lemon.cs.elte.hu/trac/lemon), and/or 
[Google OrTools](http://code.google.com/p/or-tools/)?

I did while programming in C++, and I want to share with you my experience.

## The Algorithm

The algorithm is quite simple. First partition the nodes of the input graph _G=(N,A)_ in three
sets: the sets of (1) _scanned_, (2) _reachable_, and (3) _unvisited_ nodes.
Every node has a distance label $$d_i$$ and a predecessor vertex $$p_i$$. Initially, set the 
label of the source node $$d_s=0$$, while set $$d_i=+\infty$$ for all other nodes. Moreover,
the node _s_ is placed in the set of *reachable* nodes, while all the other nodes are *unvisited*.

The algorithm proceedes as follows: select a *reachable* node _i_ with minimum distance label,
and move it in the set of *scanned* nodes, it will be never selected again. 
For each arc _(i,j)_ in the forward star of node _i_ check if node _j_ has distance label $$d_j > d_i + c_{ij}$$; 
if it is the case, update the label $$d_j = d_i + c_{ij}$$ and the predecessor vertex $$p_j=i$$.
In addition, if the node was _unvisited_, move it in the 
set of reachable nodes. If the selected node _i_ is the destination node
_t_, stop the algorithm.
Otherwise, continue by selecting the next node _i_ with minimum distance label.

The algorithm stops either when it scans the destination node _t_ or the set of reachable nodes is empty.
For the nice properties of the algorithm consult any textbook in computer science or operations research. 

At this point it should be clear why Dijkstra's algorithm is **greedy**: it always select a reachable node with
minimum distance label. It is a **dynamic programming** algorithm because it maintains the 
recursive relation 
$$d_j = \min \{d_i + c_{ij} \mid (i,j) \in A \}$$ 
for all $$j \in N$$.
If you are familiar with Linear Programming, you should recognize that the distance labels
play the role of dual variable of a flow based formulation of the shortest path problem,
and the Dijkstra's algorithm costruct a **primal** solution (i.e. a path) that satisfies the **dual**
constraints $$d_j - d_i \leq c_{ij}$$.

## Graphs and Heaps

The algorithm uses two data structures: the input graph _G_ and the set of reachable nodes _Q_.
The graph _G_ can be stored with an adjacency list, but be sure that the arcs are stored in contiguous memory,
in order to reduce the chance of cache misses when scanning the forward stars. In my implementation, I have used
a std::vector to store the forward star of each node.

The second data structure, the most important, is the priority queue _Q_.
The queue has to support three operations: *push*, *update*, and *extract-min*.
The type of priority queue used determines the worst-case complexity of the Dijkstra's algorithm.
Theoretically, the best strongly polynomial worst-case complexity is achieved via a **Fibonacci heap**. 
On road networks, the Multi Bucket heap yields a weakly polynomial worst-case complexity that
is more efficient in practice. Unfortunately, the Fibonacci Heap is a rather complex data structure,
and lazy implementations end up in using a simpler Binomial Heap.

The good news is that the Boost Library from version 1.49 has a [Heap library](http://www.boost.org/doc/libs/1_51_0/doc/html/heap.html).
This library contains several type of heaps that share a common interface: 
d-ary-heap, binomial-heap, fibonacci-heap, pairing-heap, and skew-heap.
The worst-case complexity of the basic operations are summarized in a
[nice table](http://www.boost.org/doc/libs/1_51_0/doc/html/heap/data_structures.html#heap.data_structures.data_structure_configuration)
Contrary to text-books, these heaps are ordered in non increasing order (they are max-heap instead of min-heap), that means
that the top of the heap is always the element with highest priority. For implementing Dijkstra,
where all arc lengths are non negative, this is not a problem: we can store the elements with
the distance changed in sign (sorry for the rough explanation, but if you are _really_ intrested it is better to read directly the source code).

The big advantage of **boost::heap** is that it allows to program Dijkstra once, and to compile it
with different heaps via templates.

## Benchmarking on Road Networks
Here is the point that maybe interest you the most: can we do better than well-reputed C++ graph libraries?

I have tried three graph libraries: 
[Boost Graph Library (BGL)](http://www.boost.org/doc/libs/1_51_0/libs/graph/doc/index.html), 
[COIN-OR Lemon](http://lemon.cs.elte.hu/trac/lemon), and 
[Google OrTools](http://code.google.com/p/or-tools/).
They all have a Dijkstra implementation, even if I don't know the implementation details.
As a plus, the three libraries have python wrappers (but I have not test it).
The BGL is a header only library. Lemon came after BGL.
BGL, Lemon, and my implementation use (different) Fibonacci Heaps, while I have not clear what type of priority queue is used by OrTools.

**Disclaimer**: Google OrTools is much more than a graph library: among others, it has a Constraint Programming solver with very nice
features for Large Neighborhood Search; however, we are interested here only in its Dijkstra implementation.
Constraint Programming will be the subject of another future post.

A few tests on instances taken from the last [DIMACS challenge on Shortest Path problems](http://www.dis.uniroma1.it/challenge9/download.shtml)
show the pros and cons of each implementation. Three instances are generated using the **rand** graph generator, while 10 instances are
road networks. The test are done on my late 2008 MacBookPro using the apple gcc-4.2 compiler.
All the source code, script, and even this post text, are available on a github repository.

## RAND Graphs

The first test compares the four implementations on 3 graphs with different density _d_%(that is the ratio $\frac{2m}{n(n-1)}$).
The graphs are:

1. **Rand 1**: with _n_=10000, _m_=100000, _d_=0.001%
2. **Rand 2**: with _n_=10000, _m_=1000000, _d_=0.01%
3. **Rand 3**: with _n_=10000, _m_=10000000, _d_=0.1%

For each graph, 50 queries between different pairs of source and destination nodes are performed.
The table below reports the average and standard deviations (in round brackets) query times.
The entries in bold highlight the shortest time per row.

|---------|--------------------|-----------------|----------------|----------------  
| Graph   |  MyGraph           |  BGL            |    Lemon       |   OrTools        
|:--------|:------------------:|:---------------:|:--------------:|:--------------:  
| Rand 1  | **0.0052 (0.002)** |  0.0059 (0.000) | 0.0074 (0.003) | 1.2722 (0.552)   
| Rand 2  | **0.0134 (0.005)** |  0.0535 (0.000) | 0.0706 (0.036) | 1.6128 (0.714)   
| Rand 3  | **0.0705 (0.039)** |  0.5276 (0.002) | 0.7247 (0.401) | 4.2535 (0.688)   
|---------|--------------------|-----------------|----------------|----------------  

<br>
In these tests, it looks like my implementation is the winner... wow!
Although, the true winner is the boost::heap library, since the nasty implementation details
are delegated to that library.

... but come on! These are artificial graphs: who is really interested in shortest paths on random graphs?


## Road Networks

The second test uses road networks that are **very** sparse graphs. 
We report only average computation time in seconds over 50 different pair of source-destination nodes.
We decided to leave out OrTools since it is not very performing on very sparse graphs.

This table below shows the average query time for the standard implementations that use **Fibonacci Heaps**.

|-----------------------|-----------|------------|------------|------------|--------  
| Area                  |   nodes   |  arcs      |  MyGraph   |  BGL       |  Lemon       
|:----------------------|:---------:|:----------:|:----------:|:----------:|:------:  
| Western USA           | 6,262,104 | 15,248,146 | **2.7215** |   2.7804   | 3.8181   
| Eastern USA           | 3,598,623 |  8,778,114 | 1.9425     | **1.4255** | 2.7147   
| Great Lakes           | 2,758,119 |  6,885,658 | **0.1808** |   0.8946   | 0.2602   
| California and Nevada | 1,890,815 |  4,657,742 | **0.5078** |   0.5808   | 0.7083   
| Northeast USA         | 1,524,453 |  3,897,636 | 0.6061     | **0.5662** | 0.8335   
| Northwest USA         | 1,207,945 |  2,840,208 | 0.3652     | **0.3506** | 0.5152   
| Florida               | 1,070,376 |  2,712,798 | **0.1141** |   0.2753   | 0.1574   
| Colorado              |   435,666 |  1,057,066 | 0.1423     | **0.1117** | 0.1965   
| San Francisco Bay     |   321,270 |    800,172 | 0.1721     | **0.0836** | 0.2399   
| New York City         |   264,346 |    733,846 | **0.0121** |   0.0677   | 0.0176   
|-----------------------|-----------|------------|------------|------------|--------  

<br>
From this table, BGL and my implementation are equally good, while Lemon comes after.
What happen if we use a diffent type of heap?

This second table shows the average query time for the Lemon graph library with a specialized 
[Binary Heap](http://lemon.cs.elte.hu/pub/doc/latest/a00048.html) implementation,
and my own implementation with generic **2-Heap** and **3-Heap** (binary and ternary heaps) and with a **Skew Heap**.
Note that in order to use a different heap I just modify a single line of code.

|-----------------------|-----------|------------|----------|----------|-----------|--------------  
| Area                  |   nodes   |  arcs      |  2-Heap  |  3-Heap  | Skew Heap | Lemon 2-Heap   
|:----------------------|:---------:|:----------:|:--------:|:--------:|:---------:|:------------:  
| Western USA           | 6,262,104 | 15,248,146 |  1.977   |  1.934   | 2.104     | **1.359**      
| Eastern USA           | 3,598,623 |  8,778,114 |  1.406   |  1.372   | 1.492     | **0.938**      
| Great Lakes           | 2,758,119 |  6,885,658 |  0.132   |  0.130   | 0.135     | **0.109**       
| California and Nevada | 1,890,815 |  4,657,742 |  0.361   |  0.353   | 0.372     | **0.241**       
| Northeast USA         | 1,524,453 |  3,897,636 |  0.433   |  0.421   | 0.457     | **0.287**       
| Northwest USA         | 1,207,945 |  2,840,208 |  0.257   |  0.252   | 0.256     | **0.166**       
| Florida               | 1,070,376 |  2,712,798 |  0.083   |  0.081   | 0.080     | **0.059**        
| Colorado              |   435,666 |  1,057,066 |  0.100   |  0.098   | 0.100     | **0.064**       
| San Francisco Bay     |   321,270 |    800,172 |  0.121   |  0.117   | 0.122     | **0.075**       
| New York City         |   264,346 |    733,846 |  0.009   |  0.009   | 0.009     | **0.007**        
|-----------------------|-----------|------------|----------|----------|-----------|--------------  

<br>
Mmmm... I am no longer the winner: COIN-OR Lemon is!

This is likely due to the specialized binary heap implementation of the Lemon library.
Instead, the boost::heap library has a **d-ary-heap**, that for _d=2_ is a generic binary heap.

## So what?

Dijkstra's algorithm is so beatiful because it has the _elegance of simplicity_.

Using an existing efficient heap data structure, it is easy to implement an **efficient** version of the algorithm.

However, if you have spare time, or you need to solve shortest path problems on a specific type of graphs (e.g., road networks),
you might give a try with existing graph libraries, before investing developing time in your own implementation.

All the code I have used to write this post is available. If you have any comment or criticism, do not hesitate to 
to write a comment below.

### References

1. <p> Pohl, I.
   <span class="title">Bi-directional and heuristic search in path problems</span>. 
   <span class="journal">Department of Computer Science, Stanford University</span>, 1969
   <a href="http://www.slac.stanford.edu/cgi-wrap/getdoc/slac-r-104.pdf">[pdf]</a></p>

2. <p>Sniedovich, M.
   <span class="title">Dijkstra's algorithm revisited: the dynamic programming connexion</span>.
   <span class="journal">Control and cybernetics</span> vol. 35(3), pages 599-620, 2006.
   <a href="http://oxygene.ibspan.waw.pl:3000/contents/export?filename=Sniedovich.pdf">[pdf]</a></p>

3. <p>Dantzig, G.B.
   <span class="title">Linear Programming and Extensions</span>
   Princeton University Press, Princeton, NJ, 1962.</p>

4. <p>Delling, D. and Sanders, P. and Schultes, D. and Wagner, D.
   <span class="title">Engineering route planning algorithms</span>.
   <span class="journal">Algorithmics of large and complex networks</span>
   Lecture Notes in Computer Science, Volume 5515, pages 117-139, 2009.
   <a href="http://dx.doi.org/10.1007/978-3-642-02094-0_7">[doi]</a></p>

5. <p>Goldberg, A.V. and Harrelson, C.
   <span class="title">Computing the shortest path: A search meets graph theory</span>.
   <span class="journal">Proc. of the sixteenth annual ACM-SIAM symposium on Discrete algorithms</span>, 156-165, 2005.
   <a href="http://http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.136.1062&rep=rep1&type=pdf/">[pdf]</a></p>
