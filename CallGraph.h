//
// Created by chaowyc on 3/3/2021.
//

#ifndef IFDS_CALLGRAPH_H
#define IFDS_CALLGRAPH_H

#include "Graph.h"


class CallGraph :public Graph {
public:
	vector<int> idx2function;  // vid -> function_id
//	unordered_map<int, int> idx2function;  // function_id -> vid
	unordered_map<int, int> function2idx;  // function_id -> vid
	set<int> *roots = new set<int>;
public:
	CallGraph(const string&, const string&);
	void readGraph(istream&);
	set<int>* get_roots();
	vector<int> get_leaves();
	void remove_vertex(int func_id);
	int num_vertices();
};


#endif //IFDS_CALLGRAPH_H
