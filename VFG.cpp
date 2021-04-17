//
// Created by chaowyc on 5/3/2021.
//

#include "VFG.h"
#include "progressbar.h"
#include <cstdio>

VFG::VFG(const string &graphs_folder, const vector<string> &vfg_files) {
	graph = GRA();
	vl = VertexList();

	int idx = 1;
//	cout << "Reading " << idx << "/" <<vfg_files.size() << " graphs..." << '\r';
	for(auto it = vfg_files.begin(); it != vfg_files.end(); it++, idx++){
		string vfg_path = graphs_folder + '/' + *it;
		printf("Reading %d / %lu graphs... \r", idx, vfg_files.size());
		ifstream vfg(vfg_path);
		readGraph(vfg);
		vfg.close();
	}
//	string vfg_path = graphs_folder + '/' + "114.txt";
//	cout << "Processing " << 0 << "-th graph " << endl;
//	ifstream vfg(vfg_path);
//	readGraph(vfg);
//	vfg.close();
	cout << endl;
	cout << "Reading graph finished " << endl;
}

void VFG::readGraph(istream& in) {
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

	getline(in, buf); // function id
	int self_func_id;
	istringstream (buf) >> self_func_id;
	auto *func_obj = new Function(self_func_id);
	this->function_map[self_func_id] = func_obj;
	// initialize
	n_vertices += n;
	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
//	progressbar bar(n);
	for(int i = 0; i < n; i++){
		getline(in, buf);
		strTrimRight(buf);
		idx = buf.find(':');
		sub = buf.substr(0, idx);
		istringstream(sub) >> sid;
		addVertex(sid);
		this->vl[sid].func_id = self_func_id;
		buf.erase(0, idx+2);
		while (buf.find(' ') != string::npos) {
			sub = buf.substr(0, buf.find(' '));
			istringstream(sub) >> tid;
			buf.erase(0, buf.find(' ')+1);
			addEdge(sid, tid);
		}
//		bar.update();
	}

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
		if (this->callsite_map.count(cs_id)){
			cout << "to be add cs_id: " << cs_id << " self func id: " << self_func_id << " callee func id: " << callee_func_id << endl;
			auto *previous = callsite_map[cs_id];
			cout << "previous cs_id: " << previous->id << " self func id: " << previous->self_func_id << " callee func id: " << previous->callee_func_id << endl;
		}
		assert(!this->callsite_map.count(cs_id));
		this->callsite_map[cs_id] = cs_obj;

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
			this->input2csid.insert({input, cs_id});
			this->arg2csid.insert({arg, cs_id});
			cs_obj->call_edges.insert(make_pair(input, arg));
			cs_obj->input2arg.insert({input, arg});
			cs_obj->inputs.push_back(input);
			cs_obj->args.push_back(arg);
		}
		this->n_call_edges += cs_obj->call_edges.size();
		for(int j = 0; j < num_ret_output; j++){
			getline(in, buf);
			int ret, output;
			istringstream (buf) >> ret >> output;
#ifdef VFGDEBUG
			cout << "r: " << ret << " o: " << output << endl;
#endif
			this->ret2csid.insert({ret, cs_id});
			this->output2csid.insert({output, cs_id});
			cs_obj->ret_edges.insert(make_pair(ret, output));
			cs_obj->ret2output.insert({ret, output});
			cs_obj->outputs.push_back(output);
			cs_obj->rets.push_back(ret);
		}
		this->n_ret_edges += cs_obj->ret_edges.size();
	}
}

void VFG::connectInterface() {
	for(auto & cs_it : this->callsite_map){
		CallSite* cs_obj = cs_it.second;
		for(const auto & call_edge : cs_obj->call_edges){
			int input = call_edge.first, arg = call_edge.second;
//			assert(this->vl[input].kind == NodeType::NORMAL || this->vl[input].kind == NodeType::INPUT);
//			assert(this->vl[arg].kind == NodeType::NORMAL || this->vl[arg].kind == NodeType::ARG);
			this->vl[input].kind = NodeType::INPUT;
			this->vl[arg].kind = NodeType::ARG;
			this->addEdge(input, arg);
//			if(this->hasEdge(input, arg)){
//				cout << "CS " << cs_obj->id << " Exist Call Edge " << input << "->" << arg << endl;
//			}
//			else {
//				this->addEdge(input, arg);
//				cout << "CS " << cs_obj->id << " Add Call Edge " << input << "->" << arg << endl;
//			}
		}
		for(const auto & ret_edge : cs_obj->ret_edges) {
			int ret = ret_edge.first, output = ret_edge.second;
//			assert(this->vl[ret].kind == NodeType::NORMAL || this->vl[ret].kind == NodeType::RET);
//			assert(this->vl[output].kind == NodeType::NORMAL || this->vl[output].kind == NodeType::OUTPUT);
			this->vl[ret].kind = NodeType::RET;
			this->vl[output].kind = NodeType::OUTPUT;
			this->addEdge(ret, output);
//			if(this->hasEdge(ret, output)){
//				cout << "Edge " << ret << "->" << output << " exists!" << endl;
//			}
//			else {
//				this->addEdge(ret, output);
//			}
		}
	}
}


void VFG::statistics() {
	cout << "# VFG Vertices: " << n_vertices << " # VFG Edges: " << n_edges << endl;
}

void VFG::verify() {
	for(auto & cs_it : this->callsite_map){
		CallSite* cs_obj = cs_it.second;
		for(const auto & call_edge : cs_obj->call_edges){
			int input = call_edge.first, arg = call_edge.second;
			if (this->vl[input].kind != NodeType::INPUT && this->vl[input].kind != NodeType::NORMAL){
				cout << input << " " << NodeType::INPUT << " " << this->vl[input].kind << endl;
			}

			if (this->vl[arg].kind != NodeType::ARG && this->vl[arg].kind != NodeType::NORMAL){
				cout << arg << " " << NodeType::ARG << " " << this->vl[arg].kind << endl;
			}
			assert(this->vl[input].kind == NodeType::NORMAL || this->vl[input].kind == NodeType::INPUT);
			this->vl[input].kind = NodeType::INPUT;
			this->vl[arg].kind = NodeType::ARG;
			this->addEdge(input, arg);
		}
		for(const auto & ret_edge : cs_obj->ret_edges) {
			int ret = ret_edge.first, output = ret_edge.second;
			if (this->vl[ret].kind != NodeType::RET && this->vl[ret].kind != NodeType::NORMAL){
				cout << ret << " " << NodeType::RET << " " << this->vl[ret].kind << endl;
			}
			if (this->vl[output].kind != NodeType::OUTPUT && this->vl[output].kind != NodeType::NORMAL){
				cout << output << " " << NodeType::OUTPUT << " " << this->vl[output].kind << endl;
			}
			this->addEdge(ret, output);
//			if(this->hasEdge(ret, output)){
//				cout << "Edge " << ret << "->" << output << " exists!" << endl;
//			}
//			else {
//				this->addEdge(ret, output);
//			}
		}
	}
}

void VFG::print_ret2csid() {
	for(const auto & it : ret2csid){
		cout << "ret: " << it.first << " cs_id: " << it.second << endl;
	}
}

void VFG::postProcessing() {
	for(const auto & it : callsite_map){
		auto *cs_obj = it.second;
#ifdef CSDEBUG
		cout << " cs_id: " << cs_obj->id << " callee func id: " << cs_obj->callee_func_id << endl;
#endif
		Function *func_obj = nullptr;
		assert(function_map[cs_obj->callee_func_id]);
		func_obj = function_map[cs_obj->callee_func_id];
//		if(!this->function_map.count(cs_obj->callee_func_id)){
//			func_obj = new Function(cs_obj->callee_func_id);
//			assert(func_obj);
//			this->function_map[cs_obj->callee_func_id] = func_obj;
//		} else {
//			func_obj = function_map[cs_obj->callee_func_id];
//		}

		assert(func_obj);
		for(const auto & in_it: cs_obj->inputs){
			assert(this->vl[in_it].func_id == cs_obj->self_func_id);
			this->vl[in_it].func_id = cs_obj->self_func_id;
#ifdef CSDEBUG
			cout << "" << endl;
#endif
		}
		for(const auto & ar_it: cs_obj->args){
			assert(this->vl[ar_it].func_id == cs_obj->callee_func_id);
			this->vl[ar_it].func_id = cs_obj->callee_func_id;
			func_obj->args.insert(ar_it);
		}
		for(const auto & out_it:cs_obj->outputs){
			assert(this->vl[out_it].func_id == cs_obj->self_func_id);
			this->vl[out_it].func_id = cs_obj->self_func_id;
		}
		for(const auto & ret_it:cs_obj->rets){
			assert(this->vl[ret_it].func_id == cs_obj->callee_func_id);
			this->vl[ret_it].func_id = cs_obj->callee_func_id;
			func_obj->rets.insert(ret_it);
		}
	}
}

void VFG::print_func() {
	cout << "----- Function ------" << endl;
	for(const auto & it : this->function_map){
		auto *func_obj = it.second;
		cout << "Func id: " << func_obj->id << endl;

		for(const auto & ait: func_obj->args){
			cout << "arg: " << ait << " its func_id: " << this->vl[ait].func_id << endl;
		}
	}
}

void VFG::print_node() {
	cout << "-----Node: function id -----" << endl;
	for(int i = 0; i < this->num_vertices(); i++){
		cout << i << " func_id: " << this->vl[i].func_id << " type: " << this->vl[i].kind << endl;
	}
}

