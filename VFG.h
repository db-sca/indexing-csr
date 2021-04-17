//
// Created by chaowyc on 5/3/2021.
//

#ifndef IFDS_VFG_H
#define IFDS_VFG_H

#include "Graph.h"
#include <unordered_map>
#include "CallSite.h"
#include "Function.h"
#include <cassert>


class VFG : public Graph{
private:
	int n_call_edges = 0;
	int n_ret_edges = 0;
public:
	unordered_map<int, CallSite*> callsite_map;  // cs_id -> cs_obj
	unordered_map<int, Function*> function_map;  // func_id -> func_obj
	unordered_multimap<int, int> arg2csid;  // arg -> cs_id
	unordered_multimap<int, int> ret2csid;  // ret -> cs_id
	unordered_multimap<int, int> input2csid;  // input -> cs_id
	unordered_multimap<int, int> output2csid;  // output -> cs_id

public:
	VFG(const string &graphs_folder, const vector<string> &vfg_files);
	void readGraph(istream&);
	void connectInterface();
	void statistics();
	void verify();
	void print_ret2csid();
	void postProcessing();
	void print_func();
	void print_node();
};

#endif //IFDS_VFG_H
