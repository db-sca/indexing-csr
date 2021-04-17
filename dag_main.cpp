//
// Created by chaowyc on 3/4/2021.
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
#include "Grail.h"
#include "ReachBackbone.h"
#include "PathtreeQuery.h"
#include "PathTree.h"
#include "exception_list.h"

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
bool gen_query = true;
bool tc_test_flag = false;
int bb_epsilon = 10;
float bb_pr = 0;

static void usage() {
	cout << "\nUsage:\n"
			"	dag [-h] -n num_query -fd graphs_folder [-gen]\n"
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
		} else if (strcmp("-rq", argv[i]) == 0) {
			i++;
			reachable_query_file = argv[i++];
		} else if (strcmp("-nq", argv[i]) == 0) {
			i++;
			unreachable_query_file = argv[i++];
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
	double build_summary_duration, grail_on_ig_duration, bb_discover_duration, pt_on_bb_duration, grail_on_bb_duration;
	double pt_total_duration;
	double tr_er_duration;

	CallGraph cg(graphs_folder, cg_file);
	VFG vfg(graphs_folder, vfg_files);

	cout << "#CG Vertices: " << cg.num_vertices() << " #CGEdges: " << cg.num_edges() << endl;

	IndexingGraph ig(vfg);

	vfg.connectInterface();
	vfg.postProcessing();
	cout << "#VFG Vertices: " << vfg.num_vertices() << " #VFG Edges: " << vfg.num_edges() << endl;

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

//	ifstream in("Data/dag_test.gra");
//	Graph ig(in);

	cout << "#Ref V: " << ig.num_vertices() << " #Ref E: " << ig.num_edges() << endl;
	IndexingGraph ig_ref = IndexingGraph(ig);
	start = high_resolution_clock::now();
	cout << "\nComputing TR and ER ..." << endl;
	TransitiveReduction tr(ig);
	tr.markCNRNN();
	tr.gen_po_tree();
	tr.bottom_up();
	tr.linear_ER();
	end = high_resolution_clock::now();
	diff = end - start;
	tr_er_duration = diff.count();
	cout << "TR + ER Duration: " << tr_er_duration << " ms" << endl;
	cout << "#TR V: " << ig.num_vertices() << " #TR E: " << ig.num_edges() << endl;
	cout << "#ER V: " << tr.ER_g.vsize << " #ER E: " << tr.ER_g.num_edges() << endl;
	int vfg_size = vfg.num_vertices();
	float er_bin_size = Utils::output_bin_ig(tr.ER_g, base_addr, project_name);

	if(debug){
		assert(src != -1 && "Debug mode, src is not given !");
		assert(sink != -1 && "Debug mode, sink is not given !");

//		bool ifds_s = GraphUtil::DFSReach(ig_ref, src, Utils::idx(vfg_size, sink));
//		bool dfs_s = GraphUtil::DFSReach(ig, src, Utils::idx(vfg_size, sink));

		bool dfs_r = GraphUtil::DFSReach(ig_ref, src, Utils::idx(vfg_size, sink));
		bool tr_r = GraphUtil::DFSReach(ig, src, Utils::idx(vfg_size, sink));
		bool er_r = true;
		if (tr.ER[src] == tr.ER[Utils::idx(vfg_size, sink)]){
			er_r = false;
		} else {
			er_r = GraphUtil::DFSReach(tr.ER_g, tr.ER[src], tr.ER[Utils::idx(vfg_size, sink)]);
		}

		cout << "Ref: " << dfs_r << " TR: " << tr_r << " ER: " << er_r << endl;
		return 0;
	}

	// GRAIL
	int LABELINGTYPE = 1;
	bool POOL = false;
	int POOLSIZE = 100;
	start = high_resolution_clock::now();
	GraphUtil::topo_leveler(tr.ER_g);
	Grail grail(tr.ER_g, grail_dim, LABELINGTYPE, POOL, POOLSIZE);
	end = high_resolution_clock::now();
	diff = end - start;
	grail_on_ig_duration = diff.count();
	cout << "GRAIL Indexing Construction on IG Duration: " << diff.count() << " ms" << endl;

//	//backbone
//	int epsilon = bb_epsilon;
//	double pr = bb_pr / 100.0;
//	int level = 1;
//	int block = 5;
//	int ranktype = 2;
//	ReachBackbone rbb(tr.ER_g,epsilon - 1, pr, level); // NR
//	rbb.setBlockNum(block);
//	string filesystem = graphs_folder + "/" + project_name;
//	start = high_resolution_clock::now();
//	rbb.backboneDiscovery(ranktype, filesystem.c_str());
//	end = high_resolution_clock::now();
//	diff = end - start;
//	bb_discover_duration = diff.count();
//	cout << "Backbone Discover on IG Duration: " << diff.count() << " ms" << endl;
//
//	int bbsize = rbb.getBBsize();
//	int bbedgesize = rbb.getBBEdgesize();
//	cout << "#Backbone of IG Vertices: " << bbsize << " #Backbone of IG Edges: " << bbedgesize << endl;
//
//
//	// pathtree
//	int pt_alg_type = 1;
//	string ggfile = filesystem + "." + to_string(epsilon) + to_string((int)(pr * 1000)) + "gg";
//	string labelsfile = filesystem + "." + "index";
//	ifstream infile(ggfile);
//	ifstream cfile;
//	bool compress = false;
//	if (!infile) {
//		cout << "Error: Cannot open " << ggfile << endl;
//		return -1;
//	}
//	Graph bbgg(infile); // backbone gated graph
//	int bbggsize = bbgg.num_vertices();
//	int* bbgg_sccmap = new int[bbggsize];	// store pair of orignal vertex and corresponding vertex in merged graph
//	vector<int> bbgg_reverse_topo_sort;
//	cout << "Merging SCC of Backbone ..." << endl;
//
//	start = high_resolution_clock::now();
//	GraphUtil::mergeSCC(bbgg, bbgg_sccmap, bbgg_reverse_topo_sort);
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "Merging SCC of Backbone Duration: " << diff.count() << " ms" << endl;
//	cout << "#DAG of Backbone Vertices: " << bbgg.num_vertices() << " #DAG of Backbone Edges: " << bbgg.num_edges() << endl;
//	PathTree pt(bbgg, bbgg_reverse_topo_sort);
//
//	cout << "Constructing Pathtree(PT) Indexing ..." << endl;
//	start = high_resolution_clock::now();
//	pt.createLabels(pt_alg_type, cfile, compress);
//	end = high_resolution_clock::now();
//	diff = end - start;
//	pt_on_bb_duration = diff.count();
//	cout << "#PT Indexing Construction Duration: " << diff.count() << " ms" << endl;
//	ofstream lfile(labelsfile);
//	pt.save_labels(lfile);
//
//	// Query
//	bool mat = true;
//	Query* query = nullptr;
//	query = new PathtreeQuery(filesystem.c_str(), tr.ER_g, epsilon, pr, mat, &grail_on_bb_duration);
//	pt_total_duration = bb_discover_duration + pt_on_bb_duration + grail_on_bb_duration;

	cout << "----- " << project_name << ": Indexing Construction Summary ------" << endl;
	L_(linfo) << "----- " << project_name << ": Indexing Construction Summary ------";

	printf("\tDDG: #Vertices: %d \t#Edges: %d\n", vfg.num_vertices(), vfg.num_edges());
	L_(linfo) << "\tDDG: #Vertices: " << vfg.num_vertices() << "\t#Edges: " << vfg.num_edges();

	printf("\tTR: #Vertices: %d \t#Edges: %d\n", ig.num_vertices(), ig.num_edges());
	L_(linfo) << "\tTR: #Vertices: " << ig.num_vertices() << "\t#Edges: " << ig.num_edges();

	printf("\tER: #Vertices: %d \t#Edges: %d\n", tr.ER_g.vsize, tr.ER_g.num_edges());
	L_(linfo) << "\tER: #Vertices: " << tr.ER_g.vsize << "\t#Edges: " << tr.ER_g.num_edges();

	printf("\tSummary edge construction duration: %12.3f ms\n", build_summary_duration);
	L_(linfo) << "\tSummary edge construction duration: " << build_summary_duration << " ms. ";

	printf("\tTR + ER construction duration: %12.3f ms\n", tr_er_duration);
	L_(linfo) << "\tTR + ER construction duration: " << tr_er_duration << " ms. ";

	printf("\tGRAIL indices construction duration: %11.3f ms\n", grail_on_ig_duration);
	L_(linfo) << "\tGRAIL indices construction duration: " << grail_on_ig_duration << " ms. ";
//
//	printf("\tTotal pathtree on Backbone duration: %11.3f ms\n", pt_total_duration);
//	L_(linfo) << "\tTotal pathtree on Backbone duration: " << pt_total_duration << " ms. ";
//
//	printf("\t\tBackbone discover: %15.3f ms\n", bb_discover_duration);
//	L_(linfo) << "\t\tBackbone discover: " << bb_discover_duration << " ms. ";
//
//	printf("\t\tPathtree on backbone: %12.3f ms\n", pt_on_bb_duration + bb_discover_duration);
//	L_(linfo) << "\t\tPathtree on backbone: " << pt_on_bb_duration + bb_discover_duration << " ms. ";
//
//	printf("\t\tGRAIL on backbone: %15.3f ms\n", grail_on_bb_duration);
//	L_(linfo) << "\t\tGRAIL on backbone: " << grail_on_bb_duration << " ms. ";

	srand48(time(nullptr));
	vector<iEdge> reachable_pairs;
	vector<iEdge> unreachable_pairs;
	ExceptionList * el = nullptr;

	if (query_file_flag) {
		cout << "Query file queries test..." << endl;
		int s, t, label;
		cout << "reading queries..." << endl;
		ifstream reach_f(reachable_query_file);
		string buf;
		while (getline(reach_f, buf)) {
			istringstream(buf) >> s >> t;
			reachable_pairs.emplace_back(s, t);
		}
		reach_f.close();

		ifstream unreach_f(unreachable_query_file);
		while (getline(unreach_f, buf)) {
			istringstream(buf) >> s >> t;
			unreachable_pairs.emplace_back(s, t);
		}
		unreach_f.close();
	} else {
		cout << "Randomly Generating "<< query_num * 2 << " Queries Test..." << endl;

		int reachable_count = 0;
		int unreachable_count = 0;
		progressbar bar(query_num * 2);
		while (reachable_count < query_num || unreachable_count < query_num) {
			int s = lrand48() % vfg_size;
			int t = lrand48() % vfg_size;
			int er_s = tr.ER[s];
			int er_t = tr.ER[Utils::idx(vfg_size, t)];

			if (er_s == er_t){
				if (unreachable_count < query_num){
					unreachable_pairs.emplace_back(iEdge(s, t));
					unreachable_count ++;
					bar.update();
				}
				continue;
			}

//		if (GraphUtil::DFSReach(tr.ER_g, er_s, er_t)){
			if (grail.reachPP_lf(er_s, er_t, el)){
//			if (query->reach(er_s, er_t)){
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
			reachable_query_file = common_prefix + ".reach.query";
			ofstream reach_file(reachable_query_file);

			for(const auto & re : reachable_pairs){
				reach_file << re.first << " " << re.second << endl;
			}
			reach_file.close();
			unreachable_query_file = common_prefix + ".unreach.query";
			ofstream unreach_file(unreachable_query_file);
			for(const auto & ue : unreachable_pairs){
				unreach_file << ue.first << " " << ue.second << endl;
			}
			unreach_file.close();
		}
	}

	cout << "Command for csr:" << endl;
	L_(linfo) << "Command for csr:";

	cout << "./csr -fd " << graphs_folder << " -qf " << " -rq " << reachable_query_file << " -nq " << unreachable_query_file << endl;
	L_(linfo) << "./csr -fd " << graphs_folder << " -qf " << " -rq " << reachable_query_file << " -nq " << unreachable_query_file;

	cout << "Command for dag:" << endl;
	L_(linfo) << "Command for dag:";

	cout << "./dag -fd " << graphs_folder << " -qf " << " -rq " << reachable_query_file << " -nq " << unreachable_query_file << endl;
	L_(linfo) << "./dag -fd " << graphs_folder << " -qf " << " -rq " << reachable_query_file << " -nq " << unreachable_query_file << endl;

	float succ_num = 0;
	int it_num = 5;
	double average_reach_time[3] = { 0.0, 0.0, 0.0 };
	double average_unreach_time[3] = {0.0, 0.0, 0.0};

	double worst_reach_time[3] = {0.0, 0.0, 0.0};
	double worst_unreach_time[3] = {0.0, 0.0, 0.0};

	double best_reach_time[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	double best_unreach_time[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	for(int i = 0; i < it_num; i++){
		cout << "\n--------- It: " << i + 1 << " Reachable Queries Test in ER Graph ------------" << endl;
		L_(linfo) << "\n--------- It: " << i + 1 << " Reachable Queries Test in ER Graph ------------";

//	succ_num = 0;
//	cout << "\tBy DFS in Original Graph: " << endl;
//	start = high_resolution_clock::now();
//	for(const auto & rs : reachable_pairs){
//		int s = rs.first;
//		int t = Utils::idx(vfg_size, rs.second);
//		bool pt_r = GraphUtil::DFSReach(ig_ref, s, t);
//		if (!pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << rs.second << "] reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

		succ_num = 0;
		cout << "\tBy GRAIL in ER Graph: " << endl;
		L_(linfo) << "\tBy GRAIL: ";

		start = high_resolution_clock::now();
		for(const auto & rs : reachable_pairs){
			int s = tr.ER[rs.first];
			int t = tr.ER[Utils::idx(vfg_size, rs.second)];
			bool grail_r = grail.reachPP_lf(s, t, el);
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

//		succ_num = 0;
//		cout << "\tBy Pathtree in ER Graph: " << endl;
//		start = high_resolution_clock::now();
//		for(const auto & rs : reachable_pairs){
//			int s = tr.ER[rs.first];
//			int t = tr.ER[Utils::idx(vfg_size, rs.second)];
//			bool pt_r = query->reach(s, t);
//			if (!pt_r){
//				cout << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
//				L_(linfo) << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//			} else {
//				succ_num++;
//			}
//		}
//		end = high_resolution_clock::now();
//		diff = end - start;
//		cout << "\tTotal PT Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//		L_(linfo) << "\tTotal PT Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//		average_reach_time[1] += diff.count();
//		best_reach_time[1] = min(best_reach_time[1], diff.count());
//		worst_reach_time[1] = max(worst_reach_time[1], diff.count());

//	succ_num = 0;
//	cout << "\tBy DFS in ER Graph: " << endl;
//	start = high_resolution_clock::now();
//	for(const auto & rs : reachable_pairs){
//		int s = tr.ER[rs.first];
//		int t = tr.ER[Utils::idx(vfg_size, rs.second)];
//		bool pt_r = GraphUtil::DFSReach(tr.ER_g, s, t);
//		if (!pt_r){
//			cout << "### DFS Wrong: [" << rs.first << " " << s << "] to [" << rs.second << " " << t <<  "] ER reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] ER reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

//		if (i < 1) {
//			succ_num = 0;
//			cout << "\tBy Tabulation algorithm in DD Graph: " << endl;
//			L_(linfo) << "\tBy Tabulation algorithm in DD Graph : ";
//			start = high_resolution_clock::now();
//			for(const auto & rs : reachable_pairs){
//				int s = rs.first;
//				int t = rs.second;
//				bool ifds_r = tabulation.forward_reach_recursive(s, t);
//				if (!ifds_r){
//					cout << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r << endl;
//					L_(linfo) << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r;
//				} else {
//					succ_num++;
//				}
//			}
//			end = high_resolution_clock::now();
//			diff = end - start;
//			cout << "\tTotal Tabulation algorithm Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//			L_(linfo) << "\tTotal Tabulation algorithm Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//			average_reach_time[2] += diff.count();
//			best_reach_time[2] = min(best_reach_time[2], diff.count());
//			worst_reach_time[2] = max(worst_reach_time[2], diff.count());
//		} else {
//			average_reach_time[2] += worst_reach_time[2];
//		}

		cout << endl;
		cout << "--------- It: " << i + 1 << " Unreachable Queries Test in ER Graph ------------" << endl;
		L_(linfo) << "--------- It: " << i + 1 << " Unreachable Queries Test in ER Graph ------------";

//	succ_num = 0;
//	cout << "\tBy DFS in Original Graph: " << endl;
//	L_(linfo) << "\tBy DFS: ";
//	start = high_resolution_clock::now();
//	for(const auto & rs : unreachable_pairs){
//		int s = rs.first;
//		int t = Utils::idx(vfg_size, rs.second);
//		bool pt_r = GraphUtil::DFSReach(ig_ref, s, t);
//		if (pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

		succ_num = 0;
		cout << "\tBy GRAIL: " << endl;
		L_(linfo) << "\tBy GRAIL: ";
		start = high_resolution_clock::now();
		for(const auto & rs : unreachable_pairs){
			int s = tr.ER[rs.first];
			int t = tr.ER[Utils::idx(vfg_size, rs.second)];
			bool grail_r = true;
			if (s == t){
				grail_r = false;
			} else {
				grail_r = grail.reachPP_lf(s, t, el);
			}
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

//		succ_num = 0;
//		cout << "\tBy Pathtree: " << endl;
//		L_(linfo) << "\tBy Pathtree: ";
//		start = high_resolution_clock::now();
//		for(const auto & rs : unreachable_pairs){
//			int s = tr.ER[rs.first];
//			int t = tr.ER[Utils::idx(vfg_size, rs.second)];
//			bool pt_r = true;
//			if (s == t){
//				pt_r = false;
//			} else {
//				pt_r = query->reach(s, t);
//			}
//			if (pt_r){
//				cout << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
//				L_(linfo) << "### PT Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//			} else {
//				succ_num++;
//			}
//		}
//		end = high_resolution_clock::now();
//		diff = end - start;
//		cout << "\tTotal PT Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//		L_(linfo) << "\tTotal PT Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//		average_unreach_time[1] += diff.count();
//		best_unreach_time[1] = min(best_unreach_time[1], diff.count());
//		worst_unreach_time[1] = max(worst_unreach_time[1], diff.count());

//	succ_num = 0;
//	cout << "\tBy DFS in ER Graph: " << endl;
//	L_(linfo) << "\tBy DFS: ";
//	start = high_resolution_clock::now();
//	for(const auto & rs : unreachable_pairs){
//		int s = tr.ER[rs.first];
//		int t = tr.ER[Utils::idx(vfg_size, rs.second)];
//		bool pt_r = true;
//		if (s == t){
//			pt_r = false;
//		} else {
//			pt_r = GraphUtil::DFSReach(tr.ER_g, s, t);
//		}
//		if (pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << rs.second << " " << t << "] ER reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";

//		if (i < 1){
//			succ_num = 0;
//			cout << "\tBy Tabulation algorithm: " << endl;
//			L_(linfo) << "\tBy Tabulation algorithm: ";
//			start = high_resolution_clock::now();
//			for(const auto & rs : unreachable_pairs){
//				int s = rs.first;
//				int t = rs.second;
//				bool ifds_r = tabulation.forward_reach_recursive(s, t);
//				if (ifds_r){
//					cout << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r << endl;
//					L_(linfo) << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r;
//				} else {
//					succ_num++;
//				}
//			}
//			end = high_resolution_clock::now();
//			diff = end - start;
//			cout << "\tTotal Tabulation algorithm Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//
//			average_unreach_time[2] += diff.count();
//			best_unreach_time[2] = min(best_unreach_time[2], diff.count());
//			worst_unreach_time[2] = max(worst_unreach_time[2], diff.count());
//		} else {
//			average_unreach_time[2] += worst_unreach_time[2];
//		}


//	cout << "\n--------- Reachable Queries Test in TR Graph ------------" << endl;
//	L_(linfo) << "\n--------- Reachable Queries Test in TR Graph ------------";
//
//	succ_num = 0;
//	cout << "\tBy DFS in Original Graph: " << endl;
//	start = high_resolution_clock::now();
//	for(const auto & rs : reachable_pairs){
//		int s = rs.first;
//		int t = Utils::idx(vfg_size, rs.second);
//		bool pt_r = GraphUtil::DFSReach(ig_ref, s, t);
//		if (!pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << rs.second << "] reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//	succ_num = 0;
//	cout << "\tBy DFS in ER Graph: " << endl;
//	start = high_resolution_clock::now();
//	for(const auto & rs : reachable_pairs){
//		int s = rs.first;
//		int t = Utils::idx(vfg_size, rs.second);
//		bool pt_r = GraphUtil::DFSReach(ig, s, t);
//		if (!pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << rs.second << "] reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//	succ_num = 0;
//	cout << "\tBy Tabulation algorithm: " << endl;
//	L_(linfo) << "\tBy Tabulation algorithm: ";
//	start = high_resolution_clock::now();
//	for(const auto & rs : reachable_pairs){
//		int s = rs.first;
//		int t = rs.second;
//		bool ifds_r = tabulation.forward_reach_recursive(s, t);
//		if (!ifds_r){
//			cout << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r << endl;
//			L_(linfo) << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal Tabulation algorithm Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal Tabulation algorithm Time on " << reachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//	cout << endl;

//	cout << "--------- Unreachable Queries Test in TR Graph ------------" << endl;
//	L_(linfo) << "--------- Unreachable Queries Test in TR Graph ------------";
//
//	succ_num = 0;
//	cout << "\tBy DFS in Original Graph: " << endl;
//	L_(linfo) << "\tBy DFS: ";
//	start = high_resolution_clock::now();
//	for(const auto & rs : unreachable_pairs){
//		int s = rs.first;
//		int t = Utils::idx(vfg_size, rs.second);
//		bool pt_r = GraphUtil::DFSReach(ig_ref, s, t);
//		if (pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//	succ_num = 0;
//	cout << "\tBy DFS in Edge-removed Graph: " << endl;
//	L_(linfo) << "\tBy DFS: ";
//	start = high_resolution_clock::now();
//	for(const auto & rs : unreachable_pairs){
//		int s = rs.first;
//		int t = Utils::idx(vfg_size, rs.second);
//		bool pt_r = GraphUtil::DFSReach(ig, s, t);
//		if (pt_r){
//			cout << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r << endl;
//			L_(linfo) << "### DFS Wrong: [" << s << "] to [" << t << "] reach = " << pt_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
//	L_(linfo) << "\tTotal DFS Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
//
//	succ_num = 0;
//	cout << "\tBy Tabulation algorithm: " << endl;
//	L_(linfo) << "\tBy Tabulation algorithm: ";
//	start = high_resolution_clock::now();
//	for(const auto & rs : unreachable_pairs){
//		int s = rs.first;
//		int t = rs.second;
//		bool ifds_r = tabulation.forward_reach_recursive(s, t);
//		if (ifds_r){
//			cout << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r << endl;
//			L_(linfo) << "### Tabulation algorithm Wrong: [" << s << "] to [" << t << "] reach = " << ifds_r;
//		} else {
//			succ_num++;
//		}
//	}
//	end = high_resolution_clock::now();
//	diff = end - start;
//	cout << "\tTotal Tabulation algorithm Time on " << unreachable_pairs.size() << " queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;

	}

	cout << endl;
	//GRAIL
	cout << "GRAIL Time on " << reachable_pairs.size() << " R queries: " << endl;
	cout << "\t Best: " << best_reach_time[0] << " ms." << endl;
	cout << "\t Worst: " << worst_reach_time[0] << " ms." << endl;
	cout << "\t Average: " << average_reach_time[0] / it_num << " ms." << endl;

	L_(linfo) << "GRAIL Time on " << reachable_pairs.size() << " NR queries: ";
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

	cout << "\nDumping Memory for Storage Analysis ..." << endl;

//	cout << "Summary binary size: " << se_bin_size << " MB" << endl;
//	L_(linfo) << "Summary binary size: " << se_bin_size << " MB";

	cout << "ER indexing graph size: " << er_bin_size + se_bin_size << " MB" << endl;
	L_(linfo) << "ER indexing graph size: " << er_bin_size + se_bin_size << " MB";

	float grail_bin_size = Utils::output_bin_grail_indices(tr.ER_g, base_addr, project_name);
	cout << "GRAIL indices size: " << grail_bin_size << " MB" << endl;
	L_(linfo) << "GRAIL indices size: " << grail_bin_size << " MB";

//	float pt_bin_size = Utils::output_bin_pt(bbgg, pt, *query, base_addr, project_name);

//	cout << "PathTree related size: " << pt_bin_size << " MB" << endl;
//	L_(linfo) << "PathTree related size: " << pt_bin_size << " MB";

	cout << "Reachable queries are persisted in file: " << endl;
	cout << "\t" << reachable_query_file << endl;
	L_(linfo) << "\t" << reachable_query_file;


	cout << "Unreachable queries are persisted in file: " << endl;
	cout << "\t" << unreachable_query_file << endl;
	L_(linfo) << "\t" << unreachable_query_file;


	endLogger();
	return 0;
}