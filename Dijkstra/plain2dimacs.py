#!/usr/bin/python

import sys

doc = open(sys.argv[1], 'r')

print "c basic conversion from plain graph file"
Ls = doc.readline().split()
print "p sp ",Ls[0],Ls[1]
print "c graph contains "+Ls[0]+" nodes and "+Ls[1]+" arcs"

for line in doc.readlines():
    Ls = line.split()
    print "a",Ls[0],Ls[1],Ls[2]

