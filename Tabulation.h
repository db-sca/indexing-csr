//
// Created by chaowyc on 8/3/2021.
//

#ifndef IFDS_TABULATION_H
#define IFDS_TABULATION_H

#include <map>
#include <set>
#include <vector>
#include "Graph.h"
#include "VFG.h"
#include "CallGraph.h"
using namespace std;

typedef pair<int, int> iEdge;

class Tabulation {
private:
	set<iEdge> *path_edge;
	set<iEdge> *reachable_call_edge;
	vector<iEdge> *work_list;
	set<iEdge> *summary_edge;
	set<int> *reachable_node;
	vector<int> * reachable_node_relevant_to_demand;
	set<int> *visited_node;
	vector<int> *node_stack;
	VFG &g;
	CallGraph &cg;

public:
	Tabulation(VFG &g, CallGraph &cg);
	Tabulation(VFG &g, CallGraph &cg, bool is_backward);
	bool forward_reach(int vid, int sink);
	void propagate(int sid, int tid);

	int backward_DFS(int sid);
	void backward_reach();
	void visit(int sid);
	bool is_member_of_solution(int sid);
	void update_reachable_node(int vid);
	void update_reachable_node();
	void print_path_edge();
	void print_summary_edge();
	void print_cs_inputarg();
	void print_node_type();
	void print_reachable_node();
	void generate_queries();
	void build_full_summary(bool verbose);
	void process_leaves(int func_id);
	void process_nonleaves(int func_id);
	bool forward_reach_recursive(int src, int sink);
	bool build_summary(int src, int sink);

	void forward_reach_multi_sinks(int src, vector<int>& reached_sinks);
	void build_summary(int func_id, vector<int>& reached_sinks);
	void clear_summary();
};


#endif //IFDS_TABULATION_H
