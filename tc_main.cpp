//
// Created by chaowyc on 17/3/2021.
//
#include "Utils.h"
#include "VFG.h"
#include "Tabulation.h"
#include "progressbar.h"
#include <unistd.h>
#include <signal.h>
#include "log.h"
#include <chrono>
#include <unordered_set>
#include "cstdio"


using namespace std::chrono;

int query_num = -1;
int src = -1;
int sink = -1;
int grail_dim = 2;
string base_addr = "./MemoryOut/";
string query_file_folder = "./Data/";
string log_folder = "./Logs/";
string graphs_folder;
string reachable_query_file;
string unreachable_query_file;
bool query_file_flag = false;
bool debug = false;
bool skip_merge_scc = false;
bool gen_query = false;
int hour = 3600;  // 3600s for 1 hours
int n_hour = 6;
static void usage() {
	cout << "\nUsage:\n"
			"	tc [-h] -fd graphs_folder [-nh 6] [-qf -rq <path_to_reachable_queries> -nq <path_to_unreachable_queries>]\n"
			"Description:\n"
			"	-h\tPrint the help message.\n"
			"	-fd\tGraphs folder.\n"
   			"	-nh\tNumber of hours to timeout. 6 by default.\n"
			"	-qf\tSet the queryfile testing. Reading queries from the files specified by by -rq and -nq.\n"
			"   \t-rq\t: Path to the reachable queries file.\n"
			"   \t-nq\t: Path to the unreachable queries file.\n"
		 << endl;
}

void sig_handler(){
	printf("\nRunning OUT OF TIME ! Over %d hours !\n", n_hour);
	exit(-1);
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
		} else if(strcmp("-d", argv[i])== 0) {
			i++;
			debug = true;
		} else if (strcmp("-dim", argv[i]) == 0) {
			i++;
			grail_dim = atoi(argv[i++]);
		} else if(strcmp("-gen", argv[i])== 0) {
			i++;
			gen_query = true;
		} else if(strcmp("-nh", argv[i]) == 0){
			i++;
			n_hour = atoi(argv[i++]);
		} else if (strcmp("-rq", argv[i]) == 0) {
			i++;
			reachable_query_file = argv[i++];
		} else if (strcmp("-nq", argv[i]) == 0) {
			i++;
			unreachable_query_file = argv[i++];
		} else if(strcmp("-qf", argv[i]) == 0){
			i++;
			query_file_flag = true;
		}
		/*else {
			filename = argv[i++];
		} */
	}
}

static int index(int n, int x, int y){
	return n * x + y;
}

bool query_tc(int s, int t, unordered_map<int, unordered_set<int>> &reached_sink_map){
	auto &reached_sink = reached_sink_map[s];
	if (reached_sink.count(t)){
		return true;
	} else {
		return false;
	}
}

int main(int argc, char* argv[]){
	parse_arg(argc, argv);

	vector<string> vfg_files;
	string cg_file;
	Utils::read_directory(graphs_folder, vfg_files, cg_file);
	string project_name = cg_file.substr(0,cg_file.find('-'));
	string executor = string(argv[0]).substr(string(argv[0]).find('/') + 1);

	initLogger(log_folder, executor, project_name, query_num, linfo);
	L_(linfo) << project_name;

	CallGraph cg(graphs_folder, cg_file);
	cout << "#CG Vertices: " << cg.num_vertices() << " #CGEdges: " << cg.num_edges() << endl;
	L_(linfo) << "#CG Vertices: " << cg.num_vertices() << " #CGEdges: " << cg.num_edges();

	VFG vfg(graphs_folder, vfg_files);
	vfg.connectInterface();
	vfg.postProcessing();
	cout << "#VFG Vertices: " << vfg.num_vertices() << " #VFG Edges: " << vfg.num_edges() << endl;
	L_(linfo) << "#VFG Vertices: " << vfg.num_vertices() << " #VFG Edges: " << vfg.num_edges();

	int vfg_size = vfg.num_vertices();
//	bit_vector* tc = new bit_vector(vfg_size * vfg_size);
	bool *tc = (bool*) malloc(sizeof (bool) * vfg_size * vfg_size);

	Tabulation tabulation(vfg, cg);

	signal(SIGALRM, reinterpret_cast<void (*)(int)>(sig_handler));
	cout << "Set timeout to " << n_hour << " hours." << endl;
	L_(linfo) << "Set timeout to " << n_hour << " hours.";

	alarm(hour * n_hour);

	unordered_map<int, vector<int>> node2reached_sinks;
	unordered_map<int, unordered_set<int>> node2reached_sinks_set;
	progressbar bar(vfg_size);
	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	chrono::duration<double, std::milli> diff{};

	for (int i = 0; i < vfg_size; ++i) {
		bar.update();
		vector<int> &reached_sinks = node2reached_sinks[i];
		unordered_set<int> &reached_sinks_set = node2reached_sinks_set[i];
		tabulation.forward_reach_multi_sinks(i, reached_sinks);
		copy(reached_sinks.begin(), reached_sinks.end(), inserter(reached_sinks_set, reached_sinks_set.end()));
		if (i % 10000 == 0){
			cout << endl;
			end = high_resolution_clock::now();
			diff = end - start;
		}
	}

	end = high_resolution_clock::now();
	diff = end - start;

	cout << "Total Time: " << diff.count() << " ms" << endl;
	L_(linfo) << "Total Time: " << diff.count() << " ms";

	cout << "Dumping the TC ... " << endl;
	float bc_bin_size = Utils::output_tc(vfg, node2reached_sinks, base_addr, project_name);
	cout << "TC binary size: " << bc_bin_size << " MB. " << endl;
	L_(linfo) << "TC binary size: " << bc_bin_size << " MB. " << endl;

	cout << "Finished TC" << endl;
	L_(linfo) << "FININSHED TC";

	if (query_file_flag){
		assert(Utils::file_exists(reachable_query_file) == 0 && "reachable query doesn't exist ! Please generate it first by -gen option.");
		assert(Utils::file_exists(unreachable_query_file) == 0 && "unreachable query doesn't exist ! Please generate it first by -gen option.");

		cout << "Query file queries test..." << endl;
		int s, t;
		cout << "reading queries..." << endl;
		vector<iEdge> reachable_pairs;
		vector<iEdge> unreachable_pairs;
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
		float average_reach_time = 0.0;
		float average_ureach_time = 0.0;
		int it_time = 5;
		for (int i = 0; i < it_time; ++i) {
			float succ_num = 0.0;
			start = high_resolution_clock::now();
			for(const auto & rs : reachable_pairs){
				s = rs.first;
				t = rs.second;

				bool tc_r = query_tc(s, t, node2reached_sinks_set);
				if (!tc_r){
					cout << "### TC Wrong: [" << s << "] to [" << t << "] reach = " << tc_r << endl;
					L_(linfo) << "### TC Wrong: [" << s << "] to [" << t << "] reach = " << tc_r;
				} else {
					succ_num++;
				}
			}
			end = high_resolution_clock::now();

			diff = end - start;
			average_reach_time += diff.count();

			cout << "\tTotal TC Time on " << reachable_pairs.size() << " R queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %." << endl;
			L_(linfo) << "\tTotal TC Time on " << reachable_pairs.size() << " R queries: " << diff.count() << " ms. Success rate: " << (succ_num / reachable_pairs.size()) * 100 << " %.";
			cout << endl;

			succ_num = 0.0;
			start = high_resolution_clock::now();
			for(const auto & rs : unreachable_pairs){
				s = rs.first;
				t = rs.second;

				bool tc_r = query_tc(s, t, node2reached_sinks_set);
				if (tc_r){
					cout << "### TC Wrong: [" << s << "] to [" << t << "] reach = " << tc_r << endl;
					L_(linfo) << "### TC Wrong: [" << s << "] to [" << t << "] reach = " << tc_r;
				} else {
					succ_num++;
				}
			}
			end = high_resolution_clock::now();
			diff = end - start;
			average_ureach_time += diff.count();
			cout << "\tTotal TC Time on " << unreachable_pairs.size() << " N queries: " << diff.count() << " ms. Success rate: " << (succ_num / unreachable_pairs.size()) * 100 << " %." << endl;
			L_(linfo) << "\tTotal TC Time on " << unreachable_pairs.size() << " N queries: " << diff.count() << " ms. Success rate: " << (succ_num / unreachable_pairs.size()) * 100 << " %.";
			cout << endl;
		}

		cout << "\tAverage TC Time on " << reachable_pairs.size() << " R queries: " << average_reach_time / it_time << " ms. " << endl;
		L_(linfo) << "\tAverage TC on " << reachable_pairs.size() << " R queries: " << average_reach_time / it_time << " ms.";

		cout << "\tAverage TC Time on " << unreachable_pairs.size() << " N queries: " << average_ureach_time / it_time << " ms. " << endl;
		L_(linfo) << "\tAverage TC on " << unreachable_pairs.size() << " N queries: " << average_ureach_time / it_time << " ms.";
	}

	endLogger();
	return 0;
}