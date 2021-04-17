//
// Created by chaowyc on 7/3/2021.
//

#ifndef IFDS_CALLSITE_H
#define IFDS_CALLSITE_H

#include <unordered_map>
#include <set>
#include <vector>
using namespace std;

class CallSite {
public:
	int id;
	int is_se_build = false;
	int callee_func_id;
	int self_func_id;
	unordered_map<int, int> input2arg;
	unordered_map<int, int> ret2output;
	vector<int> args;
	vector<int> rets;
	vector<int> outputs;
	vector<int> inputs;

	set<pair<int, int>> call_edges;
	set<pair<int, int>> ret_edges;
	set<pair<int, int>> input2output;
	unordered_multimap<int, int> summary_edge;

public:
	CallSite(int id, int callee_func_id, int self_func_id);
	CallSite();
//	~CallSite();
};


#endif //IFDS_CALLSITE_H
