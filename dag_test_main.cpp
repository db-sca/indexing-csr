//
// Created by chaowyc on 7/4/2021.
//

#include "Graph.h"
#include "VFG.h"
#include "CallGraph.h"
#include "TransitiveReduction.h"
#include <unordered_set>
#include <functional>
#include "Utils.h"
#include "log.h"
#include "Tabulation.h"
#include "progressbar.h"


int query_num = 100;
int src = -1;
int sink = -1;
int grail_dim = 2;
string base_addr = "./MemoryOut/";
string query_file_folder = "./Data/queries/";
string log_folder = "./Logs/";
string graphs_folder;
bool query_file_flag = false;
bool debug = false;
bool skip_merge_scc = false;
bool gen_query = true;
bool tc_test_flag = false;
int bb_epsilon = 10;
float bb_pr = 2;

static void usage() {
	cout << "\nUsage:\n"
			"	dag [-h] -n num_query -fd graphs_folder [-skipscc] [-dim 5] [-gen]\n"
			"Description:\n"
			"	-h\tPrint the help message.\n"
			"	-n\tNumber of reachable queries and unreachable queries to be generated, 100 by default.\n"
			"	-fd\tGraphs folder.\n"
			"	-skipscc Skip the phase converting the graph into a DAG. Use only when the input is already a DAG.\n"
			"	-gen\tSave the randomly generated queries into the file named with current project name in Data/queries/ folder.\n"
		 << endl;
}

static void parse_arg(int argc, char* argv[]){
	if (argc == 1) {
		usage();
		exit(0);
	}
	int i = 1;
	while (i < argc) {
		if (strcmp("-h", argv[i]) == 0) {
			usage();
			exit(0);
		}
		if (strcmp("-n", argv[i]) == 0) {
			i++;
			query_num = atoi(argv[i++]);
		} else if (strcmp("-fd", argv[i]) == 0) {
			i++;
			graphs_folder = argv[i++];
		} else if (strcmp("-s", argv[i]) == 0){
			i++;
			src = atoi(argv[i++]);
		} else if(strcmp("-t", argv[i]) == 0){
			i++;
			sink = atoi(argv[i++]);
		} else if(strcmp("-qf", argv[i]) == 0){
			i++;
			query_file_flag = true;
		} else if(strcmp("-skip", argv[i])== 0) {
			i++;
			skip_merge_scc = true;
		}else if(strcmp("-d", argv[i])== 0) {
			i++;
			debug = true;
		}else if (strcmp("-dim", argv[i]) == 0) {
			i++;
			grail_dim = atoi(argv[i++]);
		} else if(strcmp("-gen", argv[i])== 0) {
			i++;
			gen_query = true;
		} else if(strcmp("-tc", argv[i])== 0) {
			i++;
			tc_test_flag = true;
		} else if (strcmp("-e", argv[i]) == 0) {
			i++;
			bb_epsilon = atoi(argv[i++]);
		}else if (strcmp("-pr", argv[i]) == 0) {
			i++;
			bb_pr = atof(argv[i++]);
		}
		/*else {
			filename = argv[i++];
		} */
	}
}

int main(int argc, char* argv[]) {
	parse_arg(argc, argv);
	ifstream in("Data/dag_test.gra");
	Graph ig(in);

	cout << "#V: " << ig.num_vertices() << " #E: " << ig.num_edges() << endl;

	vector<int> g_topo_sort;
	vector<int> t_topo_sort;
	TransitiveReduction tr(ig);

	tr.markCNRNN();
	tr.gen_po_tree();
	tr.bottom_up();

	tr.linear_ER();

	cout << "#V: " << ig.num_vertices() << " #E: " << ig.num_edges() << endl;
//	cout << "#Ref V: " << ig_ref.num_vertices() << " #Ref E: " << ig_ref.num_edges() << endl;
	return 0;
}