//
// Created by chaowyc on 3/4/2021.
//

#ifndef CS_INDEXING_TRANSITIVEREDUCTION_H
#define CS_INDEXING_TRANSITIVEREDUCTION_H

#include "GraphUtil.h"
#include "Graph.h"
#include <unordered_set>
#include <functional>
//#define TRDEBUG

class TransitiveReduction {
public:
	Graph &vfg;
	Graph lpm_tree;
	Graph reduced_graph;
	Graph po_tree;
	Graph ER_g;
	set<int> CN;
	vector<int> g_topo_sort;
	vector<int> flag;
	vector<int> edge;
	vector<int> ER;
	map<int, vector<int>> vid_mapper;
public:
	explicit TransitiveReduction(Graph &g);
	void construct_lpm_tree();
	void interval_labelling();
	void markCNRNN();
	void gen_po_tree();
	bool is_reducible_node(int vid, vector<pair<int, int>> &remove_edges);
	bool is_reducible_node(int vid);
	bool is_removable_RN(Graph &g_ref, int vid, set<int> &RRN);
	void find_CN_from_NonRRN(Graph &g_ref, vector<int> &NonRRN);
	void gen_dfs_spanning_tree();
	void bottom_up();
	void linear_ER();
	void divide_and_conquer(vector<set<int>> &P, int vid);
private:
	int p_id = 0;
	vector<set<int>> divide(set<int> &P_i, int vid);
	void refine(vector<int> &P, int vid);
	void process_tree_child(int src, int trg);
	void delete_redundant_edge(int src, int trg);
	void estimate_N(int vid);
	int labelling_visit(Graph &tree, int vid, vector<bool>& visited);
	bool contain(int src_start, int src_end, int trg_start, int trg_end);
	void dfs(int vid, vector<bool> &visited);
};

bool rdt_small_cmp(int v1, int v2, Graph &g);
bool small_cmp(int v1, int v2, Graph &g);
bool big_cmp(int v1, int v2, Graph &g);

#endif //CS_INDEXING_TRANSITIVEREDUCTION_H
