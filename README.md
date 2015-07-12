News: Code for eccentricity estimation from the paper [*An Evaluation of Parallel Eccentricity Estimation Algorithms on Undirected Real-World Graphs*](http://www.cs.cmu.edu/~jshun/kdd-final.pdf) has been added. 
--------

Ligra (and Ligra+): A Lightweight Graph Processing Framework for Shared Memory
======================

Organization
--------

The code for the Ligra framework is located in the ligra/ directory,
and the code for Ligra+ is located in the ligra+/ directory.  Code for
the applications is in the apps/ directory, which is where compilation
should be performed.  Example inputs are provided in the inputs/
directory. Graph utilities are provided in the utils/ directory.

Compilation
--------

Compilation is done from within the apps/ directory. The code can be
compiled for either Ligra or Ligra+. There are two Makefiles provided
in the directory (Makefile.ligra and Makefile.ligra+), one for Ligra
and one for Ligra+. The file for the desired backend should be
linked/copied into a file named "Makefile". For example:

```
$ ln -s Makefile.ligra Makefile #if using Ligra
$ ln -s Makefile.ligra+ Makefile #if using Ligra+
```

Recommended environment

* Intel icpc compiler
* g++ &gt;= 4.8.0 with support for Cilk+, 

To compile with g++ using Cilk, define the environment variable
CILK. To compile with icpc, define the environment variable MKLROOT
and make sure CILK is not defined. To compile with g++ with no
parallel support, make sure CILK, MKLROOT and OPENMP are not
defined. Using Cilk+ seems to give the best parallel performance in
our experience.
 

Alternative
* OpenMP

To compile with OpenMP, define the environment variable OPENMP and
make sure CILK and MKLROOT are not defined.

Note: OpenMP support in Ligra has not been thoroughly tested. If you
experience any errors, please send an email to [Julian
Shun](mailto:jshun@cs.cmu.edu). A known issue is that OpenMP will not
work correctly when using the experimental version of gcc 4.8.0.

If Ligra+ is used, there are three compression schemes currently
implemented that can be used---byte codes, byte codes with run-length
encoding and nibble codes. By default, the code is compiled for byte
codes with run-length encoding. To use byte codes instead, define the
environment variable BYTE, and to use nibble codes instead, define the
environment variable NIBBLE. Parallel decoding within a vertex can be
enabled by defining the environment variable PD (by default, a
vertex's edge list is decoded sequentially).

After the appropriate environment variables are set, to compile,
simply run

```
$ make -j 16  #compiles with 16 threads (thread count can be changed)
```

The following commands cleans the directory:
```
$ make clean #removes all executables
$ make cleansrc #removes all executables and linked files from the ligra/ or ligra+/ directory
```

Running code in Ligra
-------
The applications take the input graph as input as well as an optional
flag "-s" to indicate a symmetric graph.  Symmetric graphs should be
called with the "-s" flag for better performance. For example:

```
$ ./BFS -s ../inputs/rMatGraph_J_5_100
$ ./BellmanFord -s ../inputs/rMatGraph_WJ_5_100
``` 

For BFS, BC and BellmanFord, one can also pass the "-r" flag followed
by an integer to indicate the source vertex.  rMat graphs along with
other graphs can be generated with the graph generators in the utils/
directory.  By default, the applications are run four times, with
times reported for the last three runs. This can be changed by passing
the flag "-rounds" followed by an integer indicating the number of
timed runs.

On NUMA machines, adding the command "numactl -i all " when running
the program may improve performance for large graphs. For example:

```
$ numactl -i all ./BFS -s <input file>
```

Running code in Ligra+ 
-----------
When using Ligra+, graphs must first be compressed using the encoder
program provided. The encoder program takes as input a file in the
format described in the next section, as well as an output file
name. For symmetric graphs, the flag "-s" should be passed before the
filenames, and for weighted graphs, the flag "-w" should be passed
before the filenames. For example:

```
$ ./encoder -s ../inputs/rMatGraph_J_5_100 inputs/rMatGraph_J_5_100.compressed
$ ./encoder -s -w ../inputs/rMatGraph_WJ_5_100 inputs/rMatGraph_WJ_5_100.compressed
```
 
After compressing the graphs, the applications can be run in the same
manner as in Ligra. For example:

```
$ ./BFS -s ../inputs/rMatGraph_J_5_100.compressed
$ ./BellmanFord -s ../inputs/rMatGraph_WJ_5_100.compressed
``` 

Make sure that the compression method used for compilation of the
applications is consistent with the method used to compress the graph
with the encoder program.

Input Format for Ligra applications and the Ligra+ encoder
-----------
The input format of unweighted graphs should be in one of two
formats (the Ligra+ encoder currently only supports the first format).

1) The adjacency graph format from the Problem Based Benchmark Suite
 (http://www.cs.cmu.edu/~pbbs/benchmarks/graphIO.html). The adjacency
 graph format starts with a sequence of offsets one for each vertex,
 followed by a sequence of directed edges ordered by their source
 vertex. The offset for a vertex i refers to the location of the start
 of a contiguous block of out edges for vertex i in the sequence of
 edges. The block continues until the offset of the next vertex, or
 the end if i is the last vertex. All vertices and offsets are 0 based
 and represented in decimal. The specific format is as follows:

AdjacencyGraph  
&lt;n>  
&lt;m>  
&lt;o0>  
&lt;o1>  
...  
&lt;o(n-1)>  
&lt;e0>  
&lt;e1>  
...  
&lt;e(m-1)>  

This file is represented as plain text.

2) In binary format. This requires three files NAME.config, NAME.adj,
and NAME.idx, where NAME is chosen by the user. The .config file
stores the number of vertices in the graph in text format. The .idx
file stores in binary the offsets for the vertices in the CSR format
(the <o>'s above). The .adj file stores in binary the edge targets in
the CSR format (the <e>'s above).

Weighted graphs: For format (1), the weights are listed at the end of
the file (after &lt;e(m-1)>). Currently for format (2), the weights
are all set to 1.

By default, format (1) is used. To run an input with format (2), pass
the "-b" flag as a command line argument.

By default the offsets are stored as 32-bit integers, and to represent
them as 64-bit integers, compile with the variable LONG defined. By
default the vertex IDs (edge values) are stored as 32-bit integers,
and to represent them as 64-bit integers, compile with the variable
EDGELONG defined.

Graph Utilities
---------

Several graph utilities are provided in the utils/ directory and can be compiled using "make".

### Graph Generators

Three graph generators from the [PBBS
project](http://www.cs.cmu.edu/~pbbs) are provided. **rMatGraph**
generators an rMat graph (described by Chakrabarti, Zhan and Faloutsos
in *SDM '04*). The required parameters are the number of vertices and
the output file. By default the number of directed edges is set to 10
times the number of vertices, and can be changed by specifying the
"-m" flag followed by the number of edges. The default parameters are
(*a=.5*, *b=.1*, *c=.1* and *d=.3*), and can be changed by specifying
the "-a", "-b", and "-c" flags each followed by the desired
probability (*d=1-a-b-c*). The "-s" flag followed by an integer
specifies the random seed (default value of 1).  **randLocalGraph**
generators a random graph, and the required parameters are the number
of vertices (10 times the number of edges are generated by default,
and can be changed with the "-m" flag) and the output file.
**gridGraph** takes the same parameters generates a 2 or 3 dimensional
graph, specified by the "-d" flag (default value is 2). All of the
graphs are symmetrized, so the total number of edges can be up to
twice the number specified.

Examples:
```
$ ./rMatGraph 10000000 rMat_10000000
$ ./rMatGraph -a .57 -b .19 -c .19 10000000 rMat_10000000 #modify rMat parameters
$ ./randLocalGraph 10000000 rand_10000000
$ ./randLocalGraph -m 50000000 10000000 rand_10000000 #modify edge count
$ ./gridGraph 10000000 2Dgrid_10000000
$ ./gridGraph -d 3 10000000 3Dgrid_10000000 
```

### Graph Converters

**SNAPtoAdj** converts a graph in [SNAP
format](snap.stanford.edu/data/index.html) and converts it to Ligra's
adjacency graph format. The first required parameter is the input
(SNAP) file name and second required parameter is the output (Ligra)
file name. The "-s" flag may be used to symmetrize the input
file. This converter works for any format that lists the two endpoints
of each edge separated by white space per line, with lines starting
with '#' ignored.

**adjGraphAddWeights** adds random integer weights in the range
[1,...,*log<sub>2</sub>*(number of vertices)] to an unweighted Ligra
graph in adjacency graph format, and takes as input the input file
name followed by the output file name.

**adjToBinary** converts an unweighted Ligra graph in adjacency graph
format to binary format. **adjWghToBinary** converts a weighted Ligra
graph in adjacency graph format to binary format. The arguments are
the adjacency graph file name followed by the 3 binary file names
(.idx, .adj and .config). 

Examples:
```
$ ./SNAPtoAdj SNAPfile LigraFile
$ ./adjGraphAddWeights unweightedLigraFile weightedLigraFile
$ ./adjToBinary rMatGraph_J_5_100 rMatGraph_J_5_100.idx rMatGraph_J_5_100.adj rMatGraph_J_5_100.config 
$ ./adjWghToBinary rMatGraph_WJ_5_100 rMatGraph_WJ_5_100.idx rMatGraph_WJ_5_100.adj rMatGraph_WJ_5_100.config 
```


Ligra Data Structure and Functions
---------
### Data Structure

**vertexSubset**: represents a subset of vertices in the
graph. Various constructors are given in ligra.h

### Functions

**edgeMap**: takes as input 3 required arguments and 3 optional arguments:
a graph *G*, vertexSubset *V*, struct *F*, threshold argument
(optional, default threshold is *m*/20), an option in {DENSE,
DENSE_FORWARD} (optional, default value is DENSE), and a boolean
indicating whether to remove duplicates (optional, default does not
remove duplicates). It returns as output a vertexSubset Out
(see section 4 of paper for how Out is computed).

The *F* struct contains three boolean functions: update, updateAtomic
and cond.  update and updateAtomic should take two integer arguments
(corresponding to source and destination vertex). In addition,
updateAtomic should be atomic with respect to the destination
vertex. cond takes one argument corresponding to a vertex.  For the
cond function which always returns true, cond_true can be called. 

```
struct F {
  inline bool update (intT s, intT d) {
  //fill in
  }
  inline bool updateAtomic (intT s, intT d){ 
  //fill in
  }
  inline bool cond (intT d) {
  //fill in 
  }
};
```

The threshold argument determines when edgeMap switches between
edgemapSparse and edgemapDense---for a threshold value *T*, edgeMap
calls edgemapSparse if the vertex subset size plus its number of
outgoing edges is less than *T*, and otherwise calls edgemapDense.

DENSE and is a read-based version where all vertices not satisfying
Cond loop over their incoming edges and DENSE_FORWARD is a write-based
version where each frontier vertex loops over its outgoing edges. This
optimization is described in Section 4 of the paper.

Note that duplicate removal can only be avoided if updateAtomic
returns true at most once for each vertex in a call to edgeMap.

**vertexMap**: takes as input 2 arguments: a vertexSubset *V* and a
function *F* which is applied to all vertices in *V*. It does not have
a return value.

**vertexFilter**: takes as input a vertexSubset *V* and a boolean
function *F* which is applied to all vertices in *V*. It returns a
vertexSubset containing all vertices *v* in *V* such that *F(v)*
returns true.

```
struct F {
  inline bool operator () (intT i) {
  //fill in
  }
};
```

To write your own Ligra code, it would be helpful to look at the code
for the provided applications as reference.

Currently the results of the computation are not used, but the code
can be easily modified to output the results to a file.

To develop a new implementation, simply include "ligra.h" in the
implementation files. When finished, one may add it to the ALL
variable in Makefile. The function that is passed to the Ligra/Ligra+
driver is the following Compute function, which is filled in by the
user. The first argument is the graph, and second argument is a
structure containing the command line arguments, which can be
extracted using routines in parseCommandLine.h, or manually from
P.argv and P.argc.

```
template<class vertex>
void Compute(graph<vertex>& GA, commandLine P){ 

}
```

For weighted graph applications, add "#define WEIGHTED 1" before
including ligra.h.

To write a parallel for loop in your code, simply use the parallel_for
construct in place of "for".

Graph Applications
---------
Implementation files are provided in the apps/ directory: 
BFS.C (breadth-first search), BC.C (betweenness centrality), Radii.C (graph
radii estimation), Components.C (connected components), BellmanFord.C
(Bellman-Ford shortest paths), PageRank.C, PageRankDelta.C and
BFSCC.C (connected components based on BFS).


Eccentricity Estimation 
-------- 
Code for eccentricity estimation is available in the
apps/eccentricity/ directory: kBFS-Ecc.C (2 passes of multiple BFS's),
kBFS-1Phase-Ecc.C (1 pass of multiple BFS's), FM-Ecc.C (estimation
using Flajolet-Martin counters; an implementation of a variant of HADI from *TKDD
'11*), LogLog-Ecc.C (estimation using LogLog counters; an
implementation of a variant of HyperANF from *WWW '11*), RV.C (a parallel
implementation of the algorithm by Roditty and Vassilevska Williams from
*STOC '13*), CLRSTV.C (a parallel implementation of a variant of the
algorithm by Chechik, Larkin, Roditty, Schoenebeck, Tarjan, and
Vassilevska Williams from *SODA '14*), kBFS-Exact.C (exact algorithm using
multiple BFS's), TK.C (a parallel implementation of the exact
algorithm by Takes and Kosters from *Algorithms '13*),
Simple-Approx-Ecc.C (simple 2-approximation algorithm).  Follow the
same instructions as above for compilation, but from the
apps/eccentricity/ directory.

For kBFS-Ecc.C, kBFS-1Phase-Ecc.C, FM-Ecc.C, LogLog-Ecc.C, and
kBFS-Exact.C, the "-r" flag followed by an integer indicates the
maximum number of words to associate with each vertex. For all
implementations, the "-s" flag should be used as the current
implementations are designed for undirected graphs.  To output the
eccentricity estimates to a file, use the "-out" flag followed by the
name of the output file. The file format is one integer per line, with
the eccentricity estimate for vertex *i* on line *i*.

Resources  
-------- 
Julian Shun and Guy Blelloch. [*Ligra: A
Lightweight Graph Processing Framework for Shared
Memory*](http://www.cs.cmu.edu/~jshun/ligra.pdf). Proceedings of the
ACM SIGPLAN Symposium on Principles and Practice of Parallel
Programming (PPoPP), pp. 135-146, 2013.

Julian Shun, Laxman Dhulipala and Guy Blelloch. [*Smaller and Faster:
Parallel Processing of Compressed Graphs with
Ligra+*](http://www.cs.cmu.edu/~jshun/ligra+.pdf). Proceedings of the
IEEE Data Compression Conference (DCC), pp. 403-412, 2015.

Julian Shun. [*An Evaluation of Parallel Eccentricity Estimation
Algorithms on Undirected Real-World
Graphs*](http://www.cs.cmu.edu/~jshun/kdd-final.pdf). Proceedings of
the ACM SIGKDD Conference on Knowledge Discovery and Data Mining
(KDD), 2015.

