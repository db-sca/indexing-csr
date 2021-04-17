### Compile

#### Requirements

You need a linux operating system with build-essential and cmake(version>=3.14) installed.

#### Build

Go to the root directory `indexing-csr`, then run `cmake .` and `make`

```
cd indexing-csr/
cmake .
make
```

Four executable files `csr`, `tc`, `fc` and `dag` will be generated, where `csr` is integrated with our indexing method and tabulation algorithm
implementations, `tc` is the implementation of transitive closure(`tc`), `fc` is the implementation of function cloning(`fc`) 
and `dag` is the implementation of dag reduction, including transitive reduction and equivalence reduction.

### Dataset

`cs_artifact` folder under the `Data` directory contains all the datasets used in our experiment. There are 12 folders
in `cs_artifact`. Each folder is a collection of data dependence graph of functions and a call graph with `-cg.txt`
suffix.

### Run the experiment

### csr

Specifying the graphs folder of the project(`-fd`) and number of queries(`-n`), after a sequence of
operation, the duration of indexing construction, query performance, and success rate will be reported to the terminal.

```
csr -h

Usage:
	csr [-h] -n num_query -fd graphs_folder [-gen] [-qf -rq <path_to_reachable_queries> -nq <path_to_unreachable_queries>]
Description:
	-h	Print the help message.
	-n	Number of reachable queries and unreachable queries to be generated, 100 by default.
	-fd	Graphs folder.
	-skipscc Skip the phase converting the graph into a DAG. Use only when the input is already a DAG.
	-gen	Save the randomly generated queries into the file named with current project name in Data/queries/ folder.
	-qf	Set the queryfile testing. Reading queries from the files specified by by -rq and -nq.
   	-rq	: Path to the reachable queries file.
   	-nq	: Path to the unreachable queries file.

```

Here provide a sample:

```
./csr -fd Data/cs_artifact/mcf/ -n 100

.....
.....
.....
----- mcf: Indexing Construction Summary ------
        DDG: #Vertices: 22194   #Edges: 29372
        Summary edge construction duration:       86.249 ms
        GRAIL indices construction duration:     217.701 ms
        Total pathtree on Backbone duration:    1875.224 ms
                Pathtree on backbone:     1354.414 ms
                GRAIL on backbone:         520.810 ms
Randomly Generating 200 Queries Test...
[##################################################] 100%
--------- Reachable Queries Test ------------
        By GRAIL: 
        Total GRAIL Time on 100 queries: 1.88622 ms. Success rate: 100 %.
        By Pathtree: 
        Total PT Time on 100 queries: 0.828253 ms. Success rate: 100 %.
        By Tabulation algorithm: 
        Total Tabulation algorithm Time on 100 queries: 66.8495 ms. Success rate: 100 %.

--------- Unreachable Queries Test ------------
        By GRAIL: 
        Total GRAIL Time on 100 queries: 0.178227 ms. Success rate: 100 %.
        By Pathtree: 
        Total PT Time on 100 queries: 0.090584 ms. Success rate: 100 %.
        By Tabulation algorithm: 
        Total Tabulation algorithm Time on 100 queries: 34.7317 ms. Success rate: 100 %.

Dumping Memory for Storage Analysis ...
Data dependence graph size: 1.54073 MB
Indexing graph size: 0.207062 MB
GRAIL indices size: 1.35461 MB
PathTree related size: 1.07875 MB

```

### tc
After specifying the folder of the graphs(`-fd`) and timeout hours(`-nh`), the progress of transitive closure computation and is 
presented on the fly. If the total running time exceeds the timeout setup, the `tc` will raise a ALARM signal and then exit, otherwise, the
total running time and transitive closure size will be reported to the terminal.

```
./tc -h

Usage:
	tc [-h] -fd graphs_folder [-nh 6] [-qf -rq <path_to_reachable_queries> -nq <path_to_unreachable_queries>]
Description:
	-h	Print the help message.
	-fd	Graphs folder.
	-nh	Number of hours to timeout. 6 by default.
	-qf	Set the queryfile testing. Reading queries from the files specified by by -rq and -nq.
   	-rq	: Path to the reachable queries file.
   	-nq	: Path to the unreachable queries file.
```

Sample usage:

```
./tc -fd Data/cs_artifact/mcf/

cg file: mcf-cg.txt
#CG Vertices: 44 #CGEdges: 52
Reading 44 / 44 graphs...
Reading graph finished
#VFG Vertices: 22194 #VFG Edges: 29372
Set timeout to 6 hours.
[                                                  ] 0%
######################                            ] 45%
#######################     ] 90%
######] 100%Total Time: 7516.47 ms
Dumping the TC ...
TC binary size:

```

### fc
After specifying the folder of the graphs (`fc`), the progress of function cloning is presented on the fly.
The final function cloning results, function cloning duration and size are reported to the terminal.

Note that `fc` will cause serious memory blast and eats up as much memory as it can, and then may stuck your operating system.
Be careful when running it on large projects.

```
./fc -h

Usage:
        fc [-h] -fd graphs_folder 
Description:
        -h      Print the help message.
        -fd     Graphs folder.

```

Sample usage:

```
./fc -fd Data/cs_artifact/mcf/

......
......
Total inline duration: 222.829 ms
Total #Vertices: 25643 #Edges: 33723
#CG Vertices: 44 #CG Edges: 52
Persist inline results ... 
Function cloning size: 0.128643 MB. 

```

### dag

It first computes a transitive reduction(TR) of original graph and then a equivalence reduction(ER) on the resulting TR.
And testing the query performance in the final ER graph. 
By running `csr` and `dag` and the same set of queries, we can compare the query performance in the ER and the original graph.
To do this, by default `dag` saves the randomly generated testing queries for ER graph 
and output the corresponding command for running the `csr` on the saved the queries, 
you can run the command in another terminal to get the performance in original graph.

Sample usage:
```
./dag -fd Data/cs_artifact/mcf/

----- mcf: Indexing Construction Summary ------
	DDG: #Vertices: 22194 	#Edges: 29372
	TR: #Vertices: 44388 	#Edges: 79910
	ER: #Vertices: 43862 	#Edges: 78858
	Summary edge construction duration:       85.652 ms
	TR + ER construction duration:     1311.346 ms
	GRAIL indices construction duration:     179.991 ms
Randomly Generating 200 Queries Test...
[##################################################] 100%

--------- It: 1 Reachable Queries Test in ER Graph ------------
	By GRAIL in ER Graph:
	Total GRAIL Time on 100 queries: 1.1731 ms. Success rate: 100 %.

--------- It: 1 Unreachable Queries Test in ER Graph ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.068704 ms. Success rate: 100 %.

--------- It: 2 Reachable Queries Test in ER Graph ------------
	By GRAIL in ER Graph:
	Total GRAIL Time on 100 queries: 1.01518 ms. Success rate: 100 %.

--------- It: 2 Unreachable Queries Test in ER Graph ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.050128 ms. Success rate: 100 %.

--------- It: 3 Reachable Queries Test in ER Graph ------------
	By GRAIL in ER Graph:
	Total GRAIL Time on 100 queries: 1.02504 ms. Success rate: 100 %.

--------- It: 3 Unreachable Queries Test in ER Graph ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.050358 ms. Success rate: 100 %.

--------- It: 4 Reachable Queries Test in ER Graph ------------
	By GRAIL in ER Graph:
	Total GRAIL Time on 100 queries: 1.01459 ms. Success rate: 100 %.

--------- It: 4 Unreachable Queries Test in ER Graph ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.049759 ms. Success rate: 100 %.

--------- It: 5 Reachable Queries Test in ER Graph ------------
	By GRAIL in ER Graph:
	Total GRAIL Time on 100 queries: 1.01463 ms. Success rate: 100 %.

--------- It: 5 Unreachable Queries Test in ER Graph ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.049908 ms. Success rate: 100 %.

GRAIL Time on 100 R queries:
	 Best: 1.01459 ms.
	 Worst: 1.1731 ms.
	 Average: 1.04851 ms.
GRAIL Time on 100 NR queries:
	 Best: 0.049759 ms.
	 Worst: 0.068704 ms.
	 Average: 0.0537714 ms.

Dumping Memory for Storage Analysis ...
ER indexing graph size: 0.205055 MB
GRAIL indices size: 1.33856 MB
Command for csr:
./csr -fd Data/cs_artifact/mcf/ -qf  -rq ./Data/queries/dag-mcf-100-20210417-200543.490.reach.query -nq ./Data/queries/dag-mcf-100-20210417-200543.490.unreach.query
```

Run the output command for csr to get the performance on the same set of queries in the original graph:
```

./csr -fd Data/cs_artifact/mcf/ -qf  -rq ./Data/queries/dag-mcf-100-20210417-200543.490.reach.query -nq ./Data/queries/dag-mcf-100-20210417-200543.490.unreach.query

----- mcf: Indexing Construction Summary ------
	DDG: #Vertices: 22194 	#Edges: 29372
	Summary edge construction duration:       86.602 ms
	GRAIL indices construction duration:     204.765 ms
Query file queries test...
reading queries...

--------- It: 1 Reachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 1.28226 ms. Success rate: 100 %.

--------- It: 1 Unreachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.073428 ms. Success rate: 100 %.

--------- It: 2 Reachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 1.03879 ms. Success rate: 100 %.

--------- It: 2 Unreachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.052587 ms. Success rate: 100 %.

--------- It: 3 Reachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 1.03781 ms. Success rate: 100 %.

--------- It: 3 Unreachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.052091 ms. Success rate: 100 %.

--------- It: 4 Reachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 1.03873 ms. Success rate: 100 %.

--------- It: 4 Unreachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.052431 ms. Success rate: 100 %.

--------- It: 5 Reachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 1.03875 ms. Success rate: 100 %.

--------- It: 5 Unreachable Queries Test ------------
	By GRAIL:
	Total GRAIL Time on 100 queries: 0.052018 ms. Success rate: 100 %.

GRAIL Time on 100 R queries:
	 Best: 1.03781 ms.
	 Worst: 1.28226 ms.
	 Average: 1.08727 ms.
GRAIL Time on 100 NR queries:
	 Best: 0.052018 ms.
	 Worst: 0.073428 ms.
	 Average: 0.056511 ms.

Dumping Memory for Storage Analysis ...
Data dependence graph size: 1.54073 MB
Indexing graph size: 0.207062 MB
GRAIL indices size: 1.35461 MB

```