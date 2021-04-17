//
// Created by chaowyc on 12/3/2021.
//

#ifndef IFDS_INDEXINGGRAPH_H
#define IFDS_INDEXINGGRAPH_H
#include "Graph.h"
#include "VFG.h"


class IndexingGraph : public Graph{
public:
	VFG &vfg_ref;

public:
	explicit IndexingGraph(VFG & vfg);
	void makeup_summary_edges();
	void construct_partial_indexing_graph();
	int idx(int o_vid);
};


#endif //IFDS_INDEXINGGRAPH_H
