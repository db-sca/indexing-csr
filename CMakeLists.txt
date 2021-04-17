cmake_minimum_required(VERSION 3.14)
project(cs_indexing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wcpp")
SET(CMAKE_BUILD_TYPE "Debug")
set(CS_FILES
        Graph.cpp
        GraphUtil.cpp
        main.cpp
        CallGraph.cpp
        Utils.cpp Utils.h
        VFG.cpp VFG.h
        CallSite.cpp
        Tabulation.cpp
        Function.cpp
        IndexingGraph.cpp
        Query.cpp
        bit_vector.cpp
        Grail.cpp
        TCSEstimator.cpp
        ReachBackbone.cpp
        PathTree.cpp
        DWGraph.cpp
        DWGraphUtil.cpp
        DataComp.cpp
        PathTree.cpp
        progressbar.cpp
        log.h
    )

set(FC_FILES
        fc_main.cpp
        FunctionCloning.cpp
        Utils.cpp
        CallSite.cpp
        CallGraph.cpp
        Graph.cpp
        GraphUtil.cpp
        bit_vector.cpp
        progressbar.cpp
    )

set(TC_FILES
        tc_main.cpp
        Graph.cpp
        VFG.cpp
        Tabulation.cpp
        GraphUtil.cpp
        Utils.cpp
        CallGraph.cpp
        CallSite.cpp
        Function.cpp
        bit_vector.cpp
        progressbar.cpp
    )

set(DAG_FILES
        dag_main.cpp
        Graph.cpp
        GraphUtil.cpp
        bit_vector.cpp
        progressbar.cpp
        Utils.cpp
        CallGraph.cpp
        Tabulation.cpp
        Function.cpp
        CallSite.cpp
        IndexingGraph.cpp
        VFG.cpp
        TransitiveReduction.cpp
        Grail.cpp
        PathTree.cpp
        Query.cpp
        PathtreeQuery.cpp
        ReachBackbone.cpp
        exception_list.h
        TCSEstimator.cpp
        DWGraph.cpp
        DWGraphUtil.cpp
        DataComp.cpp
)

set(DAG_TEST
        dag_test_main.cpp
        Graph.cpp
        GraphUtil.cpp
        bit_vector.cpp
        progressbar.cpp
        Utils.cpp
        TransitiveReduction.cpp
)

add_executable(csr ${CS_FILES})
add_executable(fc ${FC_FILES})
add_executable(tc ${TC_FILES})
add_executable(dag ${DAG_FILES})
#add_executable(test ${DAG_TEST})
