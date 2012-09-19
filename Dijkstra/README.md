# README

This repository contains all the source code for my first blog post.

These files are:

1. **dijkstra.markdown** is the blog post
2. **dijkstra.cc** is my own implementation of Dijkstra's algorithm using boost::heap
3. **dijkstra\_bgl.cc** is based on the Boost Graph Library 
4. **dijkstra\_lemon.cc** is the COIN-OR Lemon Graph Library implementation (you can use it with both Fibonacci and Binary heap)
5. **dijkstra\_or-tools.cc** is the Google OR-Tools implementation
6. **confg.mac** is used to set the paths to the different libraries
7. **Makefile** ... you should know about it
8. **run\_tests.bash** is the bash script I have used to run all the tests (you have to set a PATH\_DATA variable if you like to use this script)
9. **results.py** is a python script to elaborate the logs files in simple text tables
10. **logs** is a directory with the details of my runs that I used to write the blog entry
11. **small.dat** a micro graph to test the everything work as it should

The graph text files are available as a unique .tar.gz file of 333MB [at this link](http://www-dimat.unipv.it/~gualandi/resources/graphs-blog.tar.gz).

