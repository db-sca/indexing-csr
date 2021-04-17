//
// Created by chaowyc on 17/3/2021.
//

#include "FunctionCloning.h"
#include "Utils.h"
#include "CallGraph.h"
#include <chrono>
#include "log.h"

using namespace std::chrono;
int query_num = -1;
int src = -1;
int sink = -1;
int grail_dim = 2;
string base_addr = "./MemoryOut/";
string query_file_folder = "./Data/";
string log_folder = "./Logs/";
string graphs_folder;
bool query_file_flag = false;
bool debug = false;
bool skip_merge_scc = false;
bool gen_query = false;

static void usage() {
	cout << "\nUsage:\n"
			" 	fc [-h] -fd graphs_folder \n"
			"Description:\n"
			"	-h\tPrint the help message.\n"
			"	-fd\tGraphs folder.\n"
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
		}
		/*else {
			filename = argv[i++];
		} */
	}
}

int main(int argc, char* argv[]){
	parse_arg(argc, argv);
	vector<string> vfg_files;
	string cg_file;
	Utils::read_directory(graphs_folder, vfg_files, cg_file);
	string project_name = cg_file.substr(0,cg_file.find('-'));

	string executor = string(argv[0]).substr(string(argv[0]).find('/') + 1);

//	initLogger("./Logs/test.txt", ldebug);
	initLogger(log_folder, executor, project_name, query_num, linfo);
//
	L_(linfo) << project_name;

	CallGraph cg(graphs_folder, cg_file);

	FunctionCloning fc(graphs_folder, vfg_files, cg);
	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	chrono::duration<double, std::milli> diff{};
	cout << "#CG Vertices: " << cg.num_vertices() << " #CG Edges: " << cg.num_edges() << endl;
	L_(linfo) << "#CG Vertices: " << cg.num_vertices() << " #CG Edges: " << cg.num_edges();

	start = high_resolution_clock::now();
	fc.bottom_up_inline();
	end = high_resolution_clock::now();
	diff = end - start;

	cout << "Total inline duration: " << diff.count() << " ms" << endl;
	L_(linfo) << "Total inline duration: " << diff.count() << " ms";

//	for(const auto & node : cg.vertices()){
//		if (cg.in_edges(node.id).size() == 0 && cg.out_edges(node.id).size() == 0){
//			cout << "single: " << cg.idx2function[node.id] << endl;
//		}
//	}

	int total_vertices = 0;
	int total_edges = 0;
	vector<FunctionalVFG*> roots_func;
	for(const auto & root : *cg.get_roots()){
		auto *func = fc.function_map[root];
		roots_func.push_back(func);
		total_vertices += func->vfg.num_vertices();
		total_edges += func->vfg.num_edges();
	}

	cout << "Total #Vertices: " << total_vertices << " #Edges: " << total_edges << endl;
	L_(linfo) << "Total #Vertices: " << total_vertices << " #Edges: " << total_edges;

	cout << "#CG Vertices: " << cg.num_vertices() << " #CG Edges: " << cg.num_edges() << endl;
	L_(linfo) << "#CG Vertices: " << cg.num_vertices() << " #CG Edges: " << cg.num_edges();

	cout << "Persist inline results ... " << endl;
	float inline_size = Utils::output_fc(roots_func, base_addr, project_name);
	cout << "Function cloning size: " << inline_size << " MB. " << endl;
	L_(linfo) << "Function cloning size: " << inline_size << " MB. " << endl;

	return 0;
}