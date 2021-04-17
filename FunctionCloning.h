//
// Created by chaowyc on 17/3/2021.
//

#ifndef IFDS_FUNCTIONCLONING_H
#define IFDS_FUNCTIONCLONING_H
#include "Graph.h"
#include "CallGraph.h"
#include "progressbar.h"
#include "CallSite.h"
#include <chrono>

using namespace std::chrono;
class FCVFG : public Graph{
public:
	FCVFG();
	FCVFG(int func_id, int size);
	int self_func_id;
	void readGraph(istream &in,
				unordered_map<int, CallSite *> &callsite_map,
				unordered_map<int, int> &ovid2vid,
				unordered_map<int, int> &vid2ovid,
				int &func_id);
};

class FunctionalVFG{
public:
	vector<int> args;
	vector<int> rets;
	int total_num_vertices = -1;
	FCVFG vfg;
	unordered_map<int, CallSite*> callsite_map;
	unordered_map<int, int> ovid2vid;
	unordered_map<int, int> vid2ovid;
	unordered_map<string, int> cs_ovid2vid;
	int func_id = -1;
	FunctionalVFG(const string& vfg_file_path);
	FunctionalVFG(int func_id);
	friend class FCVFG;
};


class FunctionCloning {
public:
	CallGraph &cg;
public:
	FunctionCloning(const string& graphs_folder, const vector<string>& vfg_files, CallGraph& cg);
	unordered_map<int, FunctionalVFG*> function_map;
	void bottom_up_inline();
	void print_func_id();
	void process_nonleaves(int func_id);
};


#endif //IFDS_FUNCTIONCLONING_H
