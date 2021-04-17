//
// Created by chaowyc on 3/3/2021.
//

#ifndef IFDS_UTILS_H
#define IFDS_UTILS_H
#include <string>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include "VFG.h"
#include "IndexingGraph.h"
#include "PathTree.h"
#include "Query.h"
#include "FunctionCloning.h"


using namespace std;

class Utils {
public:
	static void read_directory(const string& name, vector<string>& v, string& cg_file);
	static bool ends_with(const string &str, const string &suffix);
	static int idx(int n, int i);
	static float print_mem_usage();
	static float output_bin_vfg(VFG& vfg, const string& base_addr, const string& project);
	static float output_bin_ig(Graph& ig, const string& base_addr, const string& project);  // ig
	static float output_bin_summary(VFG& vfg, const string& base_addr, const string& project);
	static float output_bin_grail_indices(Graph& ig, const string &base_addr, const string &project);
	static float output_bin_pt(Graph &bbgg, PathTree &pt, Query &pt_query, const string &base_addr, const string &project); // bb + pt + non-tree + grail on bb
	static float output_tc(VFG& vfg, unordered_map<int, vector<int>> &reached_sink_map, const string &base_addr, const string &project);
	static float output_fc(vector<FunctionalVFG*> &roots, const string &base_addr, const string &project);
	static int dir_exists(const string& path);
	static int file_exists(const string& path);
//	static float physical_memory_used_by_process();
//	static float virtual_memory_used_by_process();

};


#endif //IFDS_UTILS_H
