#!/bin/bash

SOLVERS_A="dijkstra dijkstra_bgl dijkstra_lemon dijkstra_or-tools"
SOLVERS_B="dijkstra dijkstra_bgl dijkstra_lemon"
SOLVERS_C="dijkstra_binary"

PATH_DATA="/Users/stegua/MyDATA/dimacs-shortestpath"
FILES_A="rand_10000_100000.dat rand_10000_1000000.dat rand_10000_10000000.dat"
FILES_B="US-d.BAY.dat US-d.CAL.dat US-d.COL.dat US-d.E.dat US-d.FLA.dat US-d.LKS.dat US-d.NE.dat US-d.NW.dat US-d.NY.dat US-d.W.dat"

for F in ${FILES_B}; do
   for S in ${SOLVERS_C}; do
      echo ./${S} ${PATH_DATA}/$F
      ./${S} ${PATH_DATA}/$F > logs/$F-$S.log
   done;
done;
