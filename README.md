## Indexing CFL-Reachability for Context-Sensitive Data Flow Analysis

### Build

You need a linux operating system with build-essential and cmake(version>=3.14) installed. 
Clone the repo and follow the commands below to build.

```bash
$ cd indexing-csr/
$ cmake .
$ make
```

Two executable files will be generated:
* `csr` answers a reachability query via either Reps' [2014] tabulation algorithm or our indexing scheme.

* `tc` computes the transitive closure for a given graph.

### Dataset

The `Data/cs_artifact` directory contains all the datasets used in our experiment.
Each folder in `Data/cs_artifact`, e.g., `Data/cs_artifact/gcc`, contains the data dependence graph of each function in a project, e.g., `gcc`.

### Run the experiment

#### Run csr

Specifying the graphs folder of the project(`-fd`) and number of queries(`-n`), after a sequence of
operation, the duration of indexing construction, query performance, and success rate will be reported to the terminal.

Sample usage:

```
./csr -fd Data/cs_artifact/mcf/ -n 100

.....
.....
.....
----- mcf: Indexing Construction Summary ------
        # DDG Vertices: 22194   # DDG Edges: 29372
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

#### Run tc
After specifying the folder of the graphs(`-fd`) and timeout hours(`-nh`), the progress of transitive closure computation is 
presented on the fly. If the total running time exceeds the timeout, an ALARM signal will be raised to stop the program. Otherwise, the
total running time and the size of the transitive closure will be reported.

Sample usage:

```
./tc -fd Data/cs_artifact/mcf/

cg file: mcf-cg.txt
#CG Vertices: 44 #CGEdges: 52
Reading 44 / 44 graphs...
Reading graph finished
#DDG Vertices: 22194 #DDG Edges: 29372
Set timeout to 6 hours.
[##################################################] 100%
Total Time: 7516.47 ms
Dumping the TC ...
TC binary size: 20MB
```
