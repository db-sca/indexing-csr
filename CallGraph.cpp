//
// Created by chaowyc on 3/3/2021.
//

#include "CallGraph.h"
#include "assert.h"


CallGraph::CallGraph(const string &graphs_folder, const string& cg_file) {
	string cg_path = graphs_folder + '/' + cg_file;
	ifstream cg(cg_path);
	readGraph(cg);
	for(int i = 0; i < num_vertices(); i++){
		if(graph[i].inList.size() == 0 && graph[i].outList.size() != 0){
//			cout << "roots: " << idx2function[i] << endl;
			roots->insert(idx2function[i]);
		}
	}
}

void CallGraph::readGraph(istream& in) {
	string buf;
	getline(in, buf);

	strTrimRight(buf);
	if (buf != "graph_for_greach") {
		cout << "BAD FILE FORMAT!" << endl;
		exit(0);
	}

	int n;
	getline(in, buf);
	istringstream(buf) >> n;
	// initialize
	vsize = n;
	vl = VertexList(n);
	graph = GRA(n, In_OutList());

	for (int i = 0; i < n; i++){
		addVertex(i);
	}

	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
	int function_id;
	int vid = 0;
	while (getline(in, buf)) {
		strTrimRight(buf);
		idx = buf.find(":");
		sub = buf.substr(0, idx);
		istringstream(sub) >> function_id;
		if(function2idx.find(function_id) == function2idx.end()){
			function2idx.insert(make_pair(function_id, vid));
			sid = vid;
			idx2function.push_back(function_id);
			vid++;
			assert(idx2function.size() == vid);
		} else {
			sid = function2idx[function_id];
		}

		buf.erase(0, idx+2);
		while (buf.find(" ") != string::npos) {
			sub = buf.substr(0, buf.find(" "));
			istringstream(sub) >> function_id;
			if(function2idx.find(function_id) == function2idx.end()){
				function2idx.insert(make_pair(function_id, vid));
				tid = vid;
				idx2function.push_back(function_id);
				vid++;
				assert(idx2function.size() == vid);
			} else {
				tid = function2idx[function_id];
			}
			buf.erase(0, buf.find(" ")+1);
			addEdge(sid, tid);
		}
	}

//	cout << "Map size: " << function2idx.size() << endl;
//	cout << "Vec size: " << idx2function.size() << endl;
}

set<int>* CallGraph::get_roots() {
	auto *root = new set<int>;
	for(const auto & it : this->function2idx){
		if(graph[it.second].inList.size() == 0){
			root->insert(it.first);
		}
	}
	return root;
}

int CallGraph::num_vertices() {
	int size = 0;
	for(const auto & node : vl){
		if (!node.removed){
			size++;
		}
	}
	return size;
}

vector<int> CallGraph::get_leaves() {
	vector<int> leaves;
	for(const auto & it : this->function2idx){
		if(graph[it.second].outList.size() == 0 && !vl[it.second].removed){
			leaves.push_back(it.first);
		}
	}
	return leaves;
}

void CallGraph::remove_vertex(int func_id) {
	int vid = this->function2idx[func_id];
	EdgeList *preds = &graph[vid].inList;

	for(const auto & pred_it : *preds){
		auto *pred = &graph[pred_it].outList;
//		for(const auto & it : *pred){
//			cout << it << " ";
//		}
//		cout << endl;
		auto f_it = find(pred->begin(), pred->end(), vid);
		assert(f_it != pred->end());
		pred->erase(f_it);
//		for(const auto & it : *pred){
//			cout << it << " ";
//		}
//		cout << endl;
	}
	EdgeList *succs = &graph[vid].outList;
	for(const auto & succ_it : *succs){
		auto *succ = &graph[succ_it].inList;
		auto f_it = find(succ->begin(), succ->end(), vid);
		assert(f_it != succ->end());
		succ->erase(f_it);
	}
	preds->clear();
	succs->clear();
	vl[vid].removed = true;
	vsize--;

//	for (int i = 0; i < num_vertices(); i++) {
//		el = graph[i].outList;
//		for(const auto & e_it:el){
//			cout << i << "->" << e_it << endl;
//		}
//	}
}