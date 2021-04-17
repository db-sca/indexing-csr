//
// Created by chaowyc on 17/3/2021.
//

#include "FunctionCloning.h"
#include "Utils.h"

FCVFG::FCVFG() {
	graph = GRA();
	vl = VertexList();
}

FCVFG::FCVFG(int func_id, int size) {
	this->self_func_id = func_id;
	vsize = size;
	vl = VertexList(size);
	graph = GRA(size, In_OutList());
}

void FCVFG::readGraph(istream & in,
					  unordered_map<int, CallSite*> &callsite_map,
					  unordered_map<int, int> &ovid2vid,
					  unordered_map<int, int> &vid2ovid,
					  int &func_id) {
	string buf;
	getline(in, buf);

	strTrimRight(buf);
	if (buf != "graph_for_greach") {
		cout << "BAD FILE FORMAT!" << endl;
		exit(0);
	}

	int n;
	getline(in, buf);  // # vertices
	istringstream(buf) >> n;
	for (int i = 0; i < n; ++i) {
		addVertex(i);
	}
	getline(in, buf); // function id
	int self_func_id;
	istringstream (buf) >> self_func_id;
	func_id = self_func_id;
	this->self_func_id = self_func_id;
	// initialize
	n_vertices += n;
	string sub;
	int idx;
	int sid = 0;
	int o_sid = 0;
	int tid = 0;
	int o_tid = 0;
	int vid = 0;
//	progressbar bar(n);
	for(int i = 0; i < n; i++){
		getline(in, buf);
		strTrimRight(buf);
		idx = buf.find(':');
		sub = buf.substr(0, idx);
		istringstream(sub) >> o_sid;
		if (!ovid2vid.count(o_sid)){
			ovid2vid.insert({o_sid, vid});
			vid2ovid.insert({vid, o_sid});
			sid = vid;
			vid++;
		} else {
			sid = ovid2vid[o_sid];
		}
		this->vl[sid].o_vid = o_sid;
		ovid2vid[o_sid] = sid;
		this->vl[sid].func_id = self_func_id;
		buf.erase(0, idx+2);
		while (buf.find(' ') != string::npos) {
			sub = buf.substr(0, buf.find(' '));
			istringstream(sub) >> o_tid;
			if (!ovid2vid.count(o_tid)){
				ovid2vid.insert({o_tid, vid});
				vid2ovid.insert({vid, o_tid});
				tid = vid;
				vid++;
			} else {
				tid = ovid2vid[o_tid];
			}
			this->vl[tid].o_vid = o_tid;
			buf.erase(0, buf.find(' ')+1);
			addEdge(sid, tid);
		}
//		bar.update();
	}
	assert(ovid2vid.size() == vid2ovid.size());
	// Read call site
	int num_cs;
	getline(in,buf);  // empty line
//	exit(0);
//	auto *func_obj = new Function(self_func_id);
	getline(in, buf); // number of callsite
	istringstream(buf) >> num_cs;
#ifdef VFGDEBUG
	cout << "# cs: " << num_cs << endl;
#endif
	for(int i = 0; i < num_cs; i++){
		int cs_id, num_input_arg, num_ret_output;
		int callee_func_id;
		getline(in, buf);  // cs_id callee function id
		istringstream (buf) >> cs_id >> callee_func_id;
#ifdef VFGDEBUG
		cout << "cs_id: " << cs_id << " callee_func: " << callee_func_id << endl;
#endif
		auto* cs_obj = new CallSite(cs_id, callee_func_id, self_func_id);
		if (callsite_map.count(cs_id)){
			cout << "to be add cs_id: " << cs_id << " self func id: " << self_func_id << " callee func id: " << callee_func_id << endl;
			auto *previous = callsite_map[cs_id];
			cout << "previous cs_id: " << previous->id << " self func id: " << previous->self_func_id << " callee func id: " << previous->callee_func_id << endl;
		}
		assert(!callsite_map.count(cs_id));
		callsite_map[cs_id] = cs_obj;

		getline(in, buf);  // number of call-edge number of ret-edge
		istringstream (buf) >> num_input_arg >> num_ret_output;
#ifdef VFGDEBUG
		cout << "# i-a: " << num_input_arg << " # r-o: " << num_ret_output << endl;
#endif
		for (int j = 0; j < num_input_arg; j++){
			getline(in, buf);
			int input, arg;
			istringstream (buf) >> input >> arg;
#ifdef VFGDEBUG
			cout << "i: " << input << " a: " << arg << endl;
#endif
			cs_obj->call_edges.insert(make_pair(input, arg));
			cs_obj->input2arg.insert({input, arg});
			cs_obj->inputs.push_back(input);
			cs_obj->args.push_back(arg);
		}
		for(int j = 0; j < num_ret_output; j++){
			getline(in, buf);
			int ret, output;
			istringstream (buf) >> ret >> output;
#ifdef VFGDEBUG
			cout << "r: " << ret << " o: " << output << endl;
#endif
			cs_obj->ret_edges.insert(make_pair(ret, output));
			cs_obj->ret2output.insert({ret, output});
			cs_obj->outputs.push_back(output);
			cs_obj->rets.push_back(ret);
		}
	}
}


FunctionalVFG::FunctionalVFG(const string &vfg_file_path) {
	ifstream in_g(vfg_file_path);
	vfg.readGraph(in_g, this->callsite_map, this->ovid2vid, this->vid2ovid, this->func_id);
	in_g.close();
	total_num_vertices = vfg.num_vertices();
}

FunctionalVFG::FunctionalVFG(int func_id) {
	assert(func_id != -1);
	this->func_id = func_id;
}

FunctionCloning::FunctionCloning(const string &graphs_folder, const vector<string> &vfg_files, CallGraph& cg): cg(cg) {
	int idx = 1;
	for(auto it = vfg_files.begin(); it != vfg_files.end(); it++, idx++){
		string vfg_path = graphs_folder + '/' + *it;
//		printf("Reading %d / %lu graphs... \r", idx, vfg_files.size());
		auto *func_vfg = new FunctionalVFG(vfg_path);
		assert(func_vfg->func_id != -1);
		this->function_map.insert({func_vfg->func_id, func_vfg});
//		cout << "Func id: " << func_vfg->func_id << endl;
//		for(const auto & cs : func_vfg->callsite_map){
//			cout << cs.first << endl;
//			for(const auto & ia : cs.second->input2arg){
//				cout << ia.first << " " << ia.second << endl;
//			}
//		}
	}
	cout << endl;
	cout << "Reading graph finished " << endl;
}

void FunctionCloning::print_func_id() {
	for(const auto & func : function_map){
		cout << "Func id: " << func.first << endl;
		for(const auto & cs : func.second->callsite_map){
			cout << cs.first << " ";
		}
		cout << endl;
	}
}

void FunctionCloning::bottom_up_inline() {
	CallGraph *cg_copy = new CallGraph(this->cg);
	auto leaves = cg_copy->get_leaves();
//	cout << "leaves are: ";
//		for(const auto & it : leaves){
//			cout << it << " ";
//		}
//		cout << endl;
	for(const auto & leaf : leaves){
		cg_copy->remove_vertex(leaf);
	}

//	cout << "# Vertices: " << cg_copy->num_vertices() << " # Edges: " << cg_copy->num_edges() <<  endl;
	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	chrono::duration<double, std::milli> diff{};

	progressbar bar(cg_copy->num_vertices());
	while (cg_copy->num_vertices() > 0){
		auto leaves = cg_copy->get_leaves();
//		cout << "leaves are: ";
//		for(const auto & it : leaves){
//			cout << it << " ";
//		}
//		cout << endl;
		int current_num_vertices = 0;
		int current_num_edges = 0;
		for(const auto & leaf : leaves){
			// process non-leaf
			cout << "\nprocess non-leaf function: " << leaf << endl;
			start = high_resolution_clock::now();
			process_nonleaves(leaf);
			end = high_resolution_clock::now();
			diff = end - start;
			cg_copy->remove_vertex(leaf);
			cout << "\ncurrent duration: " << diff.count() << " ms" << endl;

			auto *func = function_map[leaf];
			current_num_vertices += func->vfg.num_vertices();
			current_num_edges += func->vfg.num_edges();

			bar.update();
		}
		cout << "current size: #Vertices: " << current_num_vertices << " #Edges: " << current_num_edges << endl;
	}

}

void FunctionCloning::process_nonleaves(int func_id) {
	auto *self_func = this->function_map[func_id];
#ifdef FCDEBUG
	cout << "total num1: " << self_func->total_num_vertices << " vfg size: " << self_func->vfg.num_vertices() << endl;
#endif
	int curr_total_vertices = 0;
	for(const auto & cs : self_func->callsite_map){
		int callee_func_id = cs.second->callee_func_id;
		auto *callee_func = this->function_map[callee_func_id];
		assert(callee_func->total_num_vertices != -1);
		cout << "\tcallee "<< callee_func_id << " at " << cs.first <<  " owns " << callee_func->total_num_vertices << " nodes" << endl;
		curr_total_vertices += callee_func->total_num_vertices;
	}
	self_func->total_num_vertices += curr_total_vertices;
#ifdef FCDEBUG
	cout << "total num2: " << self_func->total_num_vertices << " vfg size: " << self_func->vfg.num_vertices() << endl;
#endif

	int vid = self_func->vid2ovid.size();
#ifdef FCDEBUG
	cout << "Start of vid: " << vid << endl;
#endif
	int sid = 0;
	int tid = 0;
	int o_sid = 0;
	int o_tid = 0;
	string cs_vid;
	for(const auto & cs : self_func->callsite_map){
		auto *callee_func = this->function_map[cs.second->callee_func_id];
#ifdef FCDEBUG
		cout << cs.first << " " << cs.second->callee_func_id << endl;
#endif
 		// inline callee's graph
		auto &callee_vfg_nodes = callee_func->vfg.vertices();
		for(const auto & v : callee_func->vid2ovid){
			o_sid = v.second;
#ifdef FCDEBUG
			cout << cs.first << " " << v.first << " " << v.second << endl;
#endif
			cs_vid = to_string(cs.first) + "_" + to_string(v.first);
#ifdef FCDEBUG
			cout << "cs_vid: " << cs_vid << endl;
#endif
			if (!self_func->cs_ovid2vid.count(cs_vid)){
				self_func->cs_ovid2vid.insert({cs_vid, vid});
				self_func->vid2ovid.insert({vid, o_sid});
				sid = vid;
				self_func->vfg.addVertex(sid);
				self_func->vfg[sid].func_id = callee_func->func_id;
				self_func->vfg[sid].o_vid = o_sid;
				vid++;
			} else {
				sid = self_func->cs_ovid2vid[cs_vid];
			}
			auto &succs = callee_func->vfg.out_edges(v.first);

			for(const auto & succ : succs){
				o_tid = callee_vfg_nodes[succ].o_vid;
				cs_vid = to_string(cs.first) + "_" + to_string(succ);
				if (!self_func->cs_ovid2vid.count(cs_vid)){
					self_func->cs_ovid2vid.insert({cs_vid, vid});
					self_func->vid2ovid.insert({vid, o_tid});
					tid = vid;
					self_func->vfg.addVertex(tid);
					self_func->vfg[tid].func_id = callee_func->func_id;
					self_func->vfg[tid].o_vid = o_tid;
					vid++;
				} else {
					tid = self_func->cs_ovid2vid[cs_vid];
				}
				self_func->vfg.addEdge(sid, tid);
			}
		}

		// add interface edges
		for(const auto & ce: cs.second->call_edges){
			sid = self_func->cs_ovid2vid[to_string(cs.first) + "_" + to_string(callee_func->ovid2vid[ce.first])];
			tid = self_func->cs_ovid2vid[to_string(cs.first) + "_" + to_string(callee_func->ovid2vid[ce.second])];
			self_func->vfg.addEdge(sid, tid);
		}
		for(const auto & re: cs.second->ret_edges){
			sid = self_func->cs_ovid2vid[to_string(cs.first) + "_" + to_string(callee_func->ovid2vid[re.first])];
			tid = self_func->cs_ovid2vid[to_string(cs.first) + "_" + to_string(callee_func->ovid2vid[re.second])];
			self_func->vfg.addEdge(sid, tid);
		}
	}
#ifdef FCDEBUG
	cout << "total num3: " << self_func->total_num_vertices << " vfg size: " << self_func->vfg.num_vertices() << endl;
#endif
	assert(self_func->total_num_vertices == self_func->vfg.num_vertices());
	assert(self_func->total_num_vertices == self_func->vid2ovid.size());
}
