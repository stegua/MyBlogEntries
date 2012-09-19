#!/usr/bin/python

import sys

doc = open(sys.argv[1], 'r')

for line in doc.readlines():
    for w in line.split(' '):
        if w == 'c':
            continue
        elif w == 'p':
            L = line.split()
            print L[2],L[3]
        elif w == 'a':
            L = line.split()
            print L[1],L[2],L[3]
