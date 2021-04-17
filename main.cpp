//
// Created by chaowyc on 3/3/2021.
//

#include <iostream>
#include <cstring>
#include <ctime>
#include <ratio>
#include <chrono>
#include "Graph.h"
#include "Utils.h"
#include "CallGraph.h"
#include "VFG.h"
#include "Tabulation.h"
#include "IndexingGraph.h"
#include "GraphUtil.h"
#include "Grail.h"
#include "exception_list.h"
#include "ReachBackbone.h"
#include "PathTree.h"
#include "Query.h"
#include "PathtreeQuery.h"
#include "progressbar.h"
#include "log.h"

using namespace std::chrono;

int query_num = 100;
int src = -1;
int sink = -1;
int grail_dim = 2;
string base_addr = "./MemoryOut/";
string query_file_folder = "./Data/queries/";
string log_folder = "./Logs/";
string graphs_folder;
bool query_file_flag = false;
string reachable_query_file;
string unreachable_query_file;
bool debug = false;
bool skip_merge_scc = false;
bool gen_query = false;
bool tc_test_flag = false;
int bb_epsilon = 10;
float bb_pr = 0;
int it_num = 5;
bool not_dag_test = true;


static void usage() {
	cout << "\nUsage:\n"
			"	csr [-h] -n num_query -fd graphs_folder [-gen] [-qf -rq <path_to_reachable_queries> -nq <path_to_unreachable_queries>]\n"
			"Description:\n"
			"	-h\tPrint the help message.\n"
   			"	-n\tNumber of reachable queries and unreachable queries to be generated, 100 by default.\n"
			"	-fd\tGraphs folder.\n"
   			"	-skipscc Skip the phase converting the graph into a DAG. Use only when the input is already a DAG.\n"
	  		"	-dt\tTo compare performace with the dag reduction graph.\n"
	 		"	-gen\tSave the randomly generated queries into the file named with current project name in Data/queries/ folder.\n"
			"	-qf\tSet the queryfile testing. Reading queries from the files specified by by -rq and -nq.\n"
   			"   \t-rq\t: Path to the reachable queries file.\n"
			"   \t-nq\t: Path to the unreachable queries file.\n"
			<< endl;
}

void call_ret_edge_check(VFG &vfg, IndexingGraph &ig){
	for(const auto & cs_idt: vfg.callsite_map){
		auto *cs_obj = cs_idt.second;

		assert(cs_obj->ret_edges.size() == cs_obj->ret2output.size());
		assert(cs_obj->call_edges.size() == cs_obj->input2arg.size());

		for(const auto & ce : cs_obj->call_edges){
			assert(vfg.hasEdge(ce.first, ce.second) && "vfg doesn't have a CE");
			assert(ig.hasEdge(ig.idx(ce.first), ig.idx(ce.second)) && "ig doesn't have s CE");
		}
		for(const auto & re : cs_obj->ret_edges){
			assert(vfg.hasEdge(re.first, re.second) && "vfg doesn't have a RE");
			assert(ig.hasEdge(re.first, re.second) && "ig doesn't have s RE");
		}
		for(const auto & io : cs_obj->input2output){
//			assert(vfg.hasEdge(io.first, io.second) && "vfg doesn't have a SE");
			assert(ig.hasEdge(ig.idx(io.first), ig.idx(io.second)) && "ig doesn't have s SE");
		}
	}
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
		} else if (strcmp("-rq", argv[i]) == 0) {
			i++;
			reachable_query_file = argv[i++];
		} else if (strcmp("-nq", argv[i]) == 0) {
			i++;
			unreachable_query_file = argv[i++];
		} else if (strcmp("-dt", argv[i]) == 0) {
			i++;
			not_dag_test = false;
		}
		/*else {
			filename = argv[i++];
		} */
	}
}

int main(int argc, char* argv[]) {
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
	double build_summary_duration, grail_on_ig_duration, bb_discover_duration, pt_on_bb_duration, grail_on_bb_duration;
	double pt_total_duration;

	CallGraph cg(graphs_folder, cg_file);
	VFG vfg(graphs_folder, vfg_files);
	cout << "#CG Vertices: " << cg.num_vertices() << " #CGEdges: " << cg.num_edges() << endl;

	IndexingGraph ig(vfg);

	vfg.connectInterface();
	vfg.postProcessing();
	cout << "#VFG Vertices: " << vfg.num_vertices() << " #VFG Edges: " << vfg.num_edges() << endl;
	float ig_bin_size = Utils::output_bin_ig(ig, base_addr, project_name);

	Tabulation tabulation(vfg, cg,true);
	cout << "\nBottom-up constructing full summary edge ..." << endl;
	auto start = high_resolution_clock::now();
	tabulation.build_full_summary(false);
	auto end = high_resolution_clock::now();
	chrono::duration<double, std::milli> diff = end - start;
	build_summary_duration = diff.count();
	cout << "\nSummary Edge Construction Duration: " << diff.count() << " ms" << endl;

	float se_bin_size = Utils::output_bin_summary(vfg, base_addr, project_name);

	ig.makeup_summary_edges();
	cout << "#Indexing-Graph(IG) Vertices: " << ig.num_vertices() << " #Indexing-Graph(IG) Edges: " << ig.num_edges() << endl;

	call_ret_edge_check(vfg, ig);
	int vfg_size = vfg.num_vertices();
	int origin_igsize = ig.num_vertices();

	if(debug){
		assert(src != -1 && "Debug mode, src is not given !");
		assert(sink != -1 && "Debug mode, sink is not given !");

		bool dfs_s = GraphUtil::DFSReach(ig, src, Utils::idx(vfg_size, sink));
		bool ifds_s = tabulation.forward_reach_recursive(src, sink);
		cout << "IFDS: " << ifds_s << " DFS: " << dfs_s << endl;
		return 0;
	}

	int *sccmap = new int[origin_igsize]; // store pair of orignal vertex and corresponding vertex in merged graph
	vector<int> reverse_topo_sort;

	if (!skip_merge_scc) {
		// merge strongly connected component
		cout << "Merging strongly connected component (SCC) of IG ..." << endl;
		start = high_resolution_clock::now();
		GraphUtil::mergeSCC(ig, sccmap, reverse_topo_sort);
		end = high_resolution_clock::now();
		diff = end - start;
		cout << "Merging SCC of Indexing-Graph(IG) Duration: " << diff.count() << " ms" << endl;
		cout << "#DAG of IG: " << ig.num_vertices() << " #DAG of IG Edges:" << ig.num_edges() << endl;
	}
	else {
		GraphUtil::topological_sort(ig, reverse_topo_sort);
		for (int i = 0; i < origin_igsize; i++)
			sccmap[i] = i;
	}

	// GRAIL
	int LABELINGTYPE = 1;
	bool POOL = false;
	int POOLSIZE = 100;
	start = high_resolution_clock::now();
	GraphUtil::topo_leveler(ig);
	Grail grail(ig, grail_dim, LABELINGTYPE, POOL, POOLSIZE);
	end = high_resolution_clock::now();
	diff = end - start;
	grail_on_ig_duration = diff.count();
	cout << "GRAIL Indexing Construction on IG Duration: " << diff.count() << " ms" << endl;

	Query* query = nullptr;
	Graph bbgg;
	PathTree* pt = nullptr;

	if (not_dag_test){
		//backbone
		int epsilon = bb_epsilon;
		double pr = bb_pr / 100.0;
		int level = 1;
		int block = 5;
		int ranktype = 2;
		ReachBackbone rbb(ig,epsilon - 1, pr, level); // NR
		rbb.setBlockNum(block);
		string filesystem = graphs_folder + "/" + project_name;
		start = high_resolution_clock::now();
		rbb.backboneDiscovery(ranktype, filesystem.c_str());
		end = high_resolution_clock::now();
		diff = end - start;
		bb_discover_duration = diff.count();
		cout << "Backbone Discover on IG Duration: " << diff.count() << " ms" << endl;

		int bbsize = rbb.getBBsize();
		int bbedgesize = rbb.getBBEdgesize();
		cout << "#Backbone of IG Vertices: " << bbsize << " #Backbone of IG Edges: " << bbedgesize << endl;

		// pathtree
		int pt_alg_type = 1;
		string ggfile = filesystem + "." + to_string(epsilon) + to_string((int)(pr * 1000)) + "gg";
		string labelsfile = filesystem + "." + "index";
		ifstream infile(ggfile);
		ifstream cfile;
		bool compress = false;
		if (!infile) {
			cout << "Error: Cannot open " << ggfile << endl;
			return -1;
		}
		bbgg = Graph(infile); // backbone gated graph
		int bbggsize = bbgg.num_vertices();
		int* bbgg_sccmap = new int[bbggsize];	// store pair of orignal vertex and corresponding vertex in merged graph
		vector<int> bbgg_reverse_topo_sort;
		cout << "Merging SCC of Backbone ..." << endl;

		start = high_resolution_clock::now();
		GraphUtil::mergeSCC(bbgg, bbgg_sccmap, bbgg_reverse_topo_sort);
		end = high_resolution_clock::now();
		diff = end - start;
		cout << "Merging SCC of Backbone Duration: " << diff.count() << " ms" << endl;
		cout << "#DAG of Backbone Vertices: " << bbgg.num_vertices() << " #DAG of Backbone Edges: " << bbgg.num_edges() << endl;
		pt = new PathTree(bbgg, bbgg_reverse_topo_sort);

		cout << "Constructing Pathtree(PT) Indexing ..." << endl;
		start = high_resolution_clock::now();
		pt->createLabels(pt_alg_type, cfile, compress);
		end = high_resolution_clock::now();
		diff = end - start;
		pt_on_bb_duration = diff.count();
		cout << "#PT Indexing Construction Duration: " << diff.count() << " ms" << endl;
		ofstream lfile(labelsfile);
		pt->save_labels(lfile);

		// Query
		bool mat = true;

		query = new PathtreeQuery(filesystem.c_str(), ig, epsilon, pr, mat, &grail_on_bb_duration);
		pt_total_duration = bb_discover_duration + pt_on_bb_duration + grail_on_bb_duration;
	}

	cout << "----- " << project_name << ": Indexing Construction Summary ------" << endl;
	L_(linfo) << "----- " << project_name << ": Indexing Construction Summary ------";

	printf("\tDDG: #Vertices: %d \t#Edges: %d\n", vfg.num_vertices(), vfg.num_edges());
	L_(linfo) << "\tDDG: #Vertices: " << vfg.num_vertices() << "\t#Edges: " << vfg.num_edges();

	printf("\tSummary edge construction duration: %12.3f ms\n", build_summary_duration);
	L_(linfo) << "\tSummary edge construction duration: " << build_summary_duration << " ms. ";

	printf("\tGRAIL indices construction duration: %11.3f ms\n", grail_on_ig_duration);
	L_(linfo) << "\tGRAIL indices construction duration: " << grail_on_ig_duration << " ms. ";

	if (not_dag_test){
		printf("\tTotal pathtree on Backbone duration: %11.3f ms\n", pt_total_duration);
		L_(linfo) << "\tTotal pathtree on Backbone duration: " << pt_total_duration << " ms. ";
//
//	printf("\t\tBackbone discover: %15.3f ms\n", bb_discover_duration);
//	L_(linfo) << "\t\tBackbone discover: " << bb_discover_duration << " ms. ";

		printf("\t\tPathtree on backbone: %12.3f ms\n", pt_on_bb_duration + bb_discover_duration);
		L_(linfo) << "\t\tPathtree on backbone: " << pt_on_bb_duration + bb_discover_duration << " ms. ";

		printf("\t\tGRAIL on backbone: %15.3f ms\n", grail_on_bb_duration);
		L_(linfo) << "\t\tGRAIL on backbone: " << grail_on_bb_duration << " ms. ";
	}


	srand48(time(nullptr));
	vector<iEdge> reachable_pairs;
	vector<iEdge> unreachable_pairs;
	ExceptionList * el = nullptr;

	if (query_file_flag){
		cout << "Query file queries test..." << endl;
		int s, t, label;
		cout << "reading queries..." << endl;
		ifstream reach_f(reachable_query_file);
		string buf;
		while(getline(reach_f, buf)) {
			istringstream(buf) >> s >> t;
			reachable_pairs.emplace_back(s, t);
		}
		reach_f.close();

		ifstream unreach_f(unreachable_query_file);
		while(getline(unreach_f, buf)) {
			istringstream(buf) >> s >> t;
			unreachable_pairs.emplace_back(s, t);
		}
		unreach_f.close();

	} else if(tc_test_flag){
		unordered_map<int, vector<int>> node2reached_sinks;
		progressbar bar(vfg_size);
		start = high_resolution_clock::now();
		for (int i = 0; i < vfg_size; ++i) {
			bar.update();
			vector<int> &reached_sinks = node2reached_sinks[i];
			tabulation.forward_reach_multi_sinks(i, reached_sinks);
			if (i % 10000 == 0){
				cout << endl;
			}
		}
		end = high_resolution_clock::now();
		diff = end - start;
		cout << "TC Time: " << diff.count() << " ms" << endl;
		cout << "TC Testing ..." << endl;
		progressbar test_bar(vfg_size * vfg_size);
		for (int i = 0; i < vfg_size; ++i) {
			auto &reached_sinks = node2reached_sinks[i];
			set<int> reached_sets(reached_sinks.begin(), reached_sinks.end());
			for (int j = 0; j < vfg_size; ++j) {
				if (reached_sets.count(j)){
					test_bar.update();
					if (!GraphUtil::DFSReach(ig, i, sccmap[Utils::idx(vfg_size, j)])){
						cout << "### Reach DFS Wrong: [" << i << "] to [" << j << "] reach = " << 0 << endl;
					}
				} else{
					test_bar.update();
					if (GraphUtil::DFSReach(ig, i, sccmap[Utils::idx(vfg_size, j)])){
						cout << "### Unreach DFS Wrong: [" << i << "] to [" << j << "] reach = " << 1 << endl;
					}
				}
			}
		}
		exit(0);
	} else {
		cout << "Randomly Generating "<< query_num * 2 << " Queries Test..." << endl;

		int reachable_count = 0;
		int unreachable_count = 0;
		progressbar bar(query_num * 2);
//		auto roots = cg.get_roots();
//		set<int> single;
//		for(const auto & node : cg.vertices()){
//			if (cg.in_edges(node.id).size() == 0 && cg.out_edges(node.id).size() == 0){
//				cout << "single: " << cg.idx2function[node.id] << endl;
//				single.insert(cg.idx2function[node.id]);
//				roots->erase(cg.idx2function[node.id]);
//			}
//		}
//		for (const auto & r :  *roots) {
//			cout << "root: " << r << endl;
//		}
//
//		int s, t;
//		int src_func_id;
//		int sink_func_id;
//		int r_count = 0;
//		int nr_count = 0;
//		while (reachable_count < query_num || unreachable_count < query_num){
//			do {
//				s = lrand48() % vfg_size;
//				src_func_id = vfg[s].func_id;
//				assert(src_func_id != -1);
//			} while (!roots->count(src_func_id));
//			assert(roots->count(src_func_id));
//
//			do {
//				t = lrand48() % vfg_size;
//				sink_func_id = vfg[t].func_id;
//				assert(sink_func_id != -1);
//			} while(roots->count(sink_func_id));
//			assert(!roots->count(sink_func_id));
//
//			if (query->reach(sccmap[s], sccmap[Utils::idx(vfg_size, t)])){
//				if (reachable_count < query_num){
//					reachable_pairs.emplace_back(iEdge(s, t));
//					reachable_count ++;
//					bar.update();
//				}
//				r_count++;
//			} else {
//				if (unreachable_count < query_num){
//					unreachable_pairs.emplace_back(iEdge(s, t));
//					unreachable_count ++;
//					bar.update();
//				}
//				nr_count++;
//			}
//		}
//		cout << "r_count: " << r_count << " nr_count: " << nr_count << endl;
		while (reachable_count < query_num || unreachable_count < query_num) {
			int s = lrand48() % vfg_size;
			int t = lrand48() % vfg_size;
//			cout << "gen: " << s << "->" << t << " ?" << endl;
//			if (tabulation.forward_reach_recursive(s, t)){
//				if (reachable_count < query_num){
//					reachable_pairs.emplace_back(iEdge(s, t));
//					reachable_count ++;
//					bar.update();
//				}
//			} else {
//				if (unreachable_count < query_num){
//					unreachable_pairs.emplace_back(iEdge(s, t));
//					unreachable_count ++;
//					bar.update();
//				}
//			}
//			if (grail.reachPP_lf(sccmap[s], sccmap[Utils::idx(vfg_size, t)], el)){
			if (query->reach(sccmap[s], sccmap[Utils::idx(vfg_size, t)])){  // generate queries by pathtree
				if (reachable_count < query_num){
					reachable_pairs.emplace_back(iEdge(s, t));
					reachable_count ++;
					bar.update();
				}
			} else {
				if (unreachable_count < query_num){
					unreachable_pairs.emplace_back(iEdge(s, t));
					unreachable_count ++;
					bar.update();
				}
			}
		}
		if(gen_query){
			assert(Utils::dir_exists(query_file_folder) > 0);
			string common_prefix = query_file_folder + executor + "-" + project_name + "-" + to_string(query_num) + "-" + NowTime();
			ofstream reach_file(common_prefix + ".reach.query");

			for(const auto & re : reachable_pairs){
				reach_file << re.first << " " << re.second << endl;
			}
			reach_file.close();

			ofstream unreach_file(common_prefix + ".unreach.query");
			for(const auto & ue : unreachable_pairs){
				unreach_file << ue.first << " " << ue.second << endl;
			}
			unreach_file.close();
		}
	}
	float succ_num = 0;
	double average_reach_time[3] = { 0.0, 0.0, 0.0 };
	double average_unreach_time[3] = {0.0, 0.0, 0.0};

	double worst_reach_time[3] = {0.0, 0.0, 0.0};
	double worst_unreach_time[3] = {0.0, 0.0, 0.0};

	double best_reach_time[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	double best_unreach_time[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	if (not_dag_test){
		it_num = 1;
	}

	for (int i = 0; i < it_num; ++i) {
		cout << "\n--------- It: " << i + 1 << " Reachable Queries Test ------------" << endl;
		L_(linfo) << "\n--------- It: " << i + 1 << " Reachable Queries Test ------------";

		succ_num = 0;
		cout << "\tBy GRAIL: " << endl;
		L_(linfo) << "\tBy GRAIL: ";

		start = high_resolution_clock::now();
		for(const auto & rs : reachable_pairs){
			int s = sccmap[rs.first];
			int t = sccmap[Utils::idx(vfg_size, rs.second)];
			bool grail_r = grail.reachPP_lf(s,t,el);
			if (!grail_r){
				cout << "### GRAIL Wrong: [" << s << "] to [" << t << "] reach = " << grail_r << endl;
				L_(linfo) << "### GRAIL Wrong: [" << s << "] to [" << t << "] reach = " << grail_r;
			} else {
				succ_num++;
			}
		}
		end = high_resolution_clock::now();
		diff = end - start;
		cout << "\tTotal GRAIL Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
		L_(linfo) << "\tTotal GRAIL Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

		average_reach_time[0] += diff.count();
		best_reach_time[0] = min(best_reach_time[0], diff.count());
		worst_reach_time[0] = max(worst_reach_time[0], diff.count());

		if (not_dag_test){
			succ_num = 0;
			cout << "\tBy Pathtree: " << endl;
			start = high_resolution_clock::now();
			for(const auto & rs : reachable_pairs){
				int s = sccmap[rs.first];
				int t = sccmap[Utils::idx(vfg_size, rs.second)];
				bool pt_r = query->reach(s, t);
				if (!pt_r){
					cout << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
					L_(linfo) << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
				} else {
					succ_num++;
				}
			}
			end = high_resolution_clock::now();
			diff = end - start;
			cout << "\tTotal PT Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
			L_(linfo) << "\tTotal PT Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

			average_reach_time[1] += diff.count();
			best_reach_time[1] = min(best_reach_time[1], diff.count());
			worst_reach_time[1] = max(worst_reach_time[1], diff.count());

			if (i < 1){
				succ_num = 0;
				cout << "\tBy Tabulation algorithm: " << endl;
				L_(linfo) << "\tBy Tabulation algorithm: ";
				start = high_resolution_clock::now();
				for(const auto & rs : reachable_pairs){
					int s = rs.first;
					int t = rs.second;
					bool ifds_r = tabulation.forward_reach_recursive(s, t);
					if (!ifds_r){
						cout << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r << endl;
						L_(linfo) << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r;
					} else {
						succ_num++;
					}
				}
				end = high_resolution_clock::now();
				diff = end - start;
				cout << "\tTotal Tabulation algorithm Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
				L_(linfo) << "\tTotal Tabulation algorithm Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

				average_reach_time[2] += diff.count();
				best_reach_time[2] = min(best_reach_time[2], diff.count());
				worst_reach_time[2] = max(worst_reach_time[2], diff.count());

			} else {
				average_reach_time[2] += best_reach_time[2];
			}
		}

		cout << endl;
		cout << "--------- It: " << i + 1 << " Unreachable Queries Test ------------" << endl;
		L_(linfo) << "--------- It: " << i + 1 << " Unreachable Queries Test ------------";

		succ_num = 0;
		cout << "\tBy GRAIL: " << endl;
		L_(linfo) << "\tBy GRAIL: ";
		start = high_resolution_clock::now();
		for(const auto & rs : unreachable_pairs){
			int s = sccmap[rs.first];
			int t = sccmap[Utils::idx(vfg_size, rs.second)];
			bool grail_r = grail.reachPP_lf(s, t, el);
			if (grail_r){
				cout << "### GRAIL Wrong: [" << s << "] to [" << t << "] reach = " << grail_r << " ms" << endl;
				L_(linfo) << "### GRAIL Wrong: [" << s << "] to [" << t << "] reach = " << grail_r << " ms";
			} else {
				succ_num++;
			}
		}
		end = high_resolution_clock::now();
		diff = end - start;
		cout << "\tTotal GRAIL Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
		L_(linfo) << "\tTotal GRAIL Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

		average_unreach_time[0] += diff.count();
		best_unreach_time[0] = min(best_unreach_time[0], diff.count());
		worst_unreach_time[0] = max(worst_unreach_time[0], diff.count());

		if (not_dag_test){
			succ_num = 0;
			cout << "\tBy Pathtree: " << endl;
			L_(linfo) << "\tBy Pathtree: ";
			start = high_resolution_clock::now();
			for(const auto & rs : unreachable_pairs){
				int s = sccmap[rs.first];
				int t = sccmap[Utils::idx(vfg_size, rs.second)];
				bool pt_r = query->reach(s, t);
				if (pt_r){
					cout << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
					L_(linfo) << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
				} else {
					succ_num++;
				}
			}
			end = high_resolution_clock::now();
			diff = end - start;
			cout << "\tTotal PT Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
			L_(linfo) << "\tTotal PT Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

			average_unreach_time[1] += diff.count();
			best_unreach_time[1] = min(best_unreach_time[1], diff.count());
			worst_unreach_time[1] = max(worst_unreach_time[1], diff.count());

			if(i < 1){
				succ_num = 0;
				cout << "\tBy Tabulation algorithm: " << endl;
				L_(linfo) << "\tBy Tabulation algorithm: ";
				start = high_resolution_clock::now();
				for(const auto & rs : unreachable_pairs){
					int s = rs.first;
					int t = rs.second;
					bool ifds_r = tabulation.forward_reach_recursive(s, t);
					if (ifds_r){
						cout << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r << endl;
						L_(linfo) << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r;
					} else {
						succ_num++;
					}
				}
				end = high_resolution_clock::now();
				diff = end - start;
				cout << "\tTotal Tabulation algorithm Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
				L_(linfo) << "\tTotal Tabulation algorithm Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

				average_unreach_time[2] += diff.count();
				best_unreach_time[2] = min(best_unreach_time[2], diff.count());
				worst_unreach_time[2] = max(worst_unreach_time[2], diff.count());
			} else {
				average_unreach_time[2] += best_unreach_time[2];
			}
		}
	}

	cout << endl;
	//GRAIL
	if (!not_dag_test){
		cout << "GRAIL Time on " << reachable_pairs.size() << " R queries: " << endl;
		cout << "\t Best: " << best_reach_time[0] << " ms." << endl;
		cout << "\t Worst: " << worst_reach_time[0] << " ms." << endl;
		cout << "\t Average: " << average_reach_time[0] / it_num << " ms." << endl;

		L_(linfo) << "GRAIL Time on " << reachable_pairs.size() << " R queries: ";
		L_(linfo) << "\t Best: " << best_reach_time[0] << " ms.";
		L_(linfo) << "\t Worst: " << worst_reach_time[0] << " ms.";
		L_(linfo) << "\t Average: " << average_reach_time[0] / it_num << " ms.";

		cout << "GRAIL Time on " << unreachable_pairs.size() << " NR queries: " << endl;
		cout << "\t Best: " << best_unreach_time[0] << " ms." << endl;
		cout << "\t Worst: " << worst_unreach_time[0] << " ms." << endl;
		cout << "\t Average: " << average_unreach_time[0] / it_num << " ms." << endl;

		L_(linfo) << "GRAIL Time on " << unreachable_pairs.size() << " NR queries: ";
		L_(linfo) << "\t Best: " << best_unreach_time[0] << " ms.";
		L_(linfo) << "\t Worst: " << worst_unreach_time[0] << " ms.";
		L_(linfo) << "\t Average: " << average_unreach_time[0] / it_num << " ms.";
	}

//	//PT
//	cout << "PT Time on " << reachable_pairs.size() << " R queries: " << endl;
//	cout << "\t Average: " << average_reach_time[1] / it_num << " ms." << endl;
//	cout << "\t Best: " << best_reach_time[1] << " ms." << endl;
//	cout << "\t Worst: " << worst_reach_time[1] << " ms." << endl;
//
//	L_(linfo) << "PT Time on " << reachable_pairs.size() << " R queries: ";
//	L_(linfo) << "\t Average: " << average_reach_time[1] / it_num << " ms.";
//	L_(linfo) << "\t Best: " << best_reach_time[1] << " ms.";
//	L_(linfo) << "\t Worst: " << worst_reach_time[1] << " ms.";
//
//	cout << "PT Time on " << unreachable_pairs.size() << " NR queries: " << endl;
//	cout << "\t Average: " << average_unreach_time[1] / it_num << " ms." << endl;
//	cout << "\t Best: " << best_unreach_time[1] << " ms." << endl;
//	cout << "\t Worst: " << worst_unreach_time[1] << " ms." << endl;
//
//	L_(linfo) << "PT Time on " << unreachable_pairs.size() << " NR queries: ";
//	L_(linfo) << "\t Average: " << average_unreach_time[1] / it_num << " ms.";
//	L_(linfo) << "\t Best: " << best_unreach_time[1] << " ms.";
//	L_(linfo) << "\t Worst: " << worst_unreach_time[1] << " ms.";
//
//	//IFDS
//	cout << "IFDS Time on " << reachable_pairs.size() << " R queries: " << endl;
//	cout << "\t Average: " << average_reach_time[2] / it_num << " ms." << endl;
//	cout << "\t Best: " << best_reach_time[2] << " ms." << endl;
//	cout << "\t Worst: " << worst_reach_time[2] << " ms." << endl;
//
//	L_(linfo) << "IFDS Time on " << unreachable_pairs.size() << " NR queries: ";
//	L_(linfo) << "\t Average: " << average_unreach_time[2] / it_num << " ms.";
//	L_(linfo) << "\t Best: " << best_unreach_time[2] << " ms.";
//	L_(linfo) << "\t Worst: " << worst_unreach_time[2] << " ms.";
//
//	cout << "IFDS Time on " << unreachable_pairs.size() << " NR queries: " << endl;
//	cout << "\t Average: " << average_unreach_time[2] / it_num << " ms." << endl;
//	cout << "\t Best: " << best_unreach_time[2] << " ms." << endl;
//	cout << "\t Worst: " << worst_unreach_time[2] << " ms." << endl;
//
//	L_(linfo) << "IFDS Time on " << unreachable_pairs.size() << " NR queries: ";
//	L_(linfo) << "\t Average: " << average_unreach_time[2] / it_num << " ms.";
//	L_(linfo) << "\t Best: " << best_unreach_time[2] << " ms.";
//	L_(linfo) << "\t Worst: " << worst_unreach_time[2] << " ms.";


	// output
	cout << "\nDumping Memory for Storage Analysis ..." << endl;
	float vfg_bin_size = Utils::output_bin_vfg(vfg, base_addr, project_name);
	cout << "Data dependence graph size: " << vfg_bin_size << " MB" << endl;
	L_(linfo) << "Data dependence graph size: " << vfg_bin_size << " MB";

//	cout << "Summary binary size: " << se_bin_size << " MB" << endl;
//	L_(linfo) << "Summary binary size: " << se_bin_size << " MB";

	cout << "Indexing graph size: " << ig_bin_size + se_bin_size << " MB" << endl;
	L_(linfo) << "Indexing graph size: " << ig_bin_size + se_bin_size << " MB";

	float grail_bin_size = Utils::output_bin_grail_indices(ig, base_addr, project_name);
	cout << "GRAIL indices size: " << grail_bin_size << " MB" << endl;
	L_(linfo) << "GRAIL indices size: " << grail_bin_size << " MB";

	if (not_dag_test){
		float pt_bin_size = Utils::output_bin_pt(bbgg, *pt, *query, base_addr, project_name);
		cout << "PathTree related size: " << pt_bin_size << " MB" << endl;
		L_(linfo) << "PathTree related size: " << pt_bin_size << " MB";
	}

	endLogger();
	return 0;
}
