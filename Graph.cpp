#include "Graph.h"

Graph::Graph() {
	graph = GRA();
	vl = VertexList();
}

Graph::Graph(int size) {
	vsize = size;
	vl = VertexList(size);
	graph = GRA(size, In_OutList());
}

Graph::Graph(GRA& g, VertexList& vlist) {
	vsize = vlist.size();
	graph = g;
	vl = vlist;
}

Graph::Graph(istream& in) {
	readGraph(in);
}

//Graph::Graph(const string& graphs_folder) {
//	vector<string> graph_files;
//	string cg_file;
//	read_directory(graphs_folder, graph_files, cg_file);
//	string cg_file_path = graphs_folder + "/" + cg_file;
//	cout << "cg file path: " << cg_file_path << endl;
//	ifstream cg(cg_file_path);
//	if(!cg){
//		cerr << "Error when reading " << cg_file_path << endl;
//		exit(-1);
//	}
//	readCallGraph(cg);
/*	for(auto it = std::begin(graph_files); it != std::end(graph_files); ++it){
		if (ends_with(*it, "a.txt")){
			a_path = graphs_folder_path + *it;
//			std::cout << a_path << std::endl;
		} else if (ends_with(*it, "b.txt")){
			b_path = graphs_folder_path + *it;
//			std::cout << b_path << std::endl;
		} else if (ends_with(*it, "middle.txt")){
			m_path = graphs_folder_path  + *it;
//			std::cout << m_path << std::endl;
		}
	}*/
//}

Graph::~Graph() {}

void Graph::printGraph() {
	writeGraph(cout);
}

void Graph::clear() {
	vsize = 0;
	graph.clear();
	vl.clear();
}

void Graph::strTrimRight(string& str) {
	string whitespaces(" \t");
	int index = str.find_last_not_of(whitespaces);
	if (index != string::npos) 
		str.erase(index+1);
	else
		str.clear();
}

void Graph::readGraph(istream& in) {
	string buf;
	getline(in, buf);

	strTrimRight(buf);
	if (buf != "graph_for_greach") {
		cout << "BAD FILE FORMAT!" << endl;
	//	exit(0);
	}
	
	int n;
	getline(in, buf);
	istringstream(buf) >> n;
	// initialize
	vsize = n;
	vl = VertexList(n);
	graph = GRA(n, In_OutList());	

	for (int i = 0; i < n; i++)
		addVertex(i);

	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
	while (getline(in, buf)) {
		strTrimRight(buf);
		idx = buf.find(":");
		sub = buf.substr(0, idx);
		istringstream(sub) >> sid;
		buf.erase(0, idx+2);
		while (buf.find(" ") != string::npos) {
			sub = buf.substr(0, buf.find(" "));
			istringstream(sub) >> tid;
			buf.erase(0, buf.find(" ")+1);
			addEdge(sid, tid);
		}
	}
}


void Graph::writeGraph(ostream& out) {
	cout << "Graph size = " << graph.size() << endl;
	out << "graph_for_greach" << endl;
	out << vl.size() << endl;

	GRA::iterator git;
	EdgeList el;
	EdgeList::iterator eit;
	VertexList::iterator vit;
	int i = 0;
	for (i = 0; i < vl.size(); i++) {
		out << i << ": ";
		el = graph[i].outList;
		for (eit = el.begin(); eit != el.end(); eit++)
			out << (*eit) << " ";
		out << "#" << endl;
	}
/*
	cout << "** In List for graph **" << endl;
	for (i = 0; i < vl.size(); i++) {
		out << i << ": ";
		el = graph[i].inList;
		for (eit = el.begin(); eit != el.end(); eit++)
			out << (*eit) << " ";
		out << "#" << endl;
	}
*/
}

void Graph::addVertex(int vid) {
	if (vid >= vl.size()) {
		int size = vl.size();
		for (int i = 0; i < (vid-size+1); i++) {
			graph.push_back(In_OutList());
			vl.push_back(Vertex(vid + i));
		}
		vsize = vl.size();
	}

	Vertex v;
	v.id = vid;
	v.top_level = -1;
	v.visited = false;
	vl[vid] = v;

	EdgeList il = EdgeList();
	EdgeList ol = EdgeList();
	In_OutList oil = In_OutList();
	oil.inList = il;
	oil.outList = ol;
	graph[vid] = oil;
//	n_vertices++;
}

void Graph::remove_vertex(int vid) {
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
}

void Graph::addEdge(int sid, int tid) {
	if (sid >= vl.size())
		addVertex(sid);
	if (tid >= vl.size())
		addVertex(tid);
	// update edge list
	graph[tid].inList.push_back(sid);
	graph[sid].outList.push_back(tid);
	n_edges++;
}

int Graph::num_vertices() {
	return vl.size();
}

int Graph::num_edges() {
	EdgeList el;
	GRA::iterator git;
	int num = 0;
	for (git = graph.begin(); git != graph.end(); git++) {
		el = git->outList;
		num += el.size();
	}
	return num;
}

// return out edges of specified vertex
EdgeList& Graph::out_edges(int src) {
	return graph[src].outList;
}

// return in edges of specified vertex
EdgeList& Graph::in_edges(int trg) {
	return graph[trg].inList;
}

int Graph::out_degree(int src) {
	return graph[src].outList.size();
}

int Graph::in_degree(int trg) {
	return graph[trg].inList.size();
}

// get roots of graph (root is zero in_degree vertex)
vector<int> Graph::getRoots() {
	vector<int> roots;
	GRA::iterator git;
	int i = 0;
	for (git = graph.begin(), i = 0; git != graph.end(); git++, i++) {
		if (git->inList.size() == 0)
			roots.push_back(i);
	}
	
	return roots;
}

vector<int> Graph::get_leaves() {
	vector<int> leaves;
	for(const auto v : vl){
		if (graph[v.id].outList.empty() && !v.removed){
			leaves.emplace_back(v.id);
		}
	}
	return leaves;
}

// check whether the edge from src to trg is in the graph
bool Graph::hasEdge(int src, int trg) {
	EdgeList el = graph[src].outList;
	EdgeList::iterator ei;
	for (ei = el.begin(); ei != el.end(); ei++)
		if ((*ei) == trg)
			return true;
	return false;

}

// return vertex list of graph
VertexList& Graph::vertices() {
	return vl;
}

Graph& Graph::operator=(const Graph& g) {
	if (this != &g) {
		graph = g.graph;
		vl = g.vl;
		vsize = g.vsize;
	}
	return *this;
}

// get a specified vertex property
Vertex& Graph::operator[](const int vid) {
	return vl[vid];
}

Graph::Graph(hash_map<int,vector<int> >& inlist, hash_map<int,vector<int> >& outlist) {
	vsize = inlist.size();
	cout << "inlist size: " << inlist.size() << endl;
	cout << "outlist size: " << outlist.size() << endl;
	vl = VertexList(vsize);
	graph = GRA(vsize, In_OutList());
	for (int i = 0; i < vsize; i++)
		addVertex(i);
	cout << "inlist size: " << inlist.size() << endl;
	cout << "outlist size: " << outlist.size() << endl;
	hash_map<int,vector<int> >::iterator hit, hit1;
	hash_map<int,int> indexmap;
	vector<int> vec;
	vector<int>::iterator vit;
	int k;
	for (hit = inlist.begin(), k = 0; hit != inlist.end(); hit++,k++) {
		indexmap[hit->first] = k;
	}
	cout << "k: " << k << endl;
	for (hit = inlist.begin(), hit1 = outlist.begin(), k = 0; hit != inlist.end(); hit++, hit1++, k++) {
//		cout << hit1->first << " " << k << endl;
		vec = hit->second;
		for (vit = vec.begin(); vit != vec.end(); vit++)
			graph[k].inList.push_back(indexmap[*vit]);
		vec = hit1->second;
		for (vit = vec.begin(); vit != vec.end(); vit++)
			graph[k].outList.push_back(indexmap[*vit]);
	}
}

void Graph::extract(hash_map<int,vector<int> >& inlist, hash_map<int,vector<int> >& outlist) {
	for (int i = 0; i < vl.size(); i++) {
		inlist[i] = graph[i].inList;
		outlist[i] = graph[i].outList;
	}
//	printMap(inlist,outlist);
}

// for test
void Graph::printMap(hash_map<int,vector<int> >& inlist, hash_map<int,vector<int> >& outlist) {
	cout << "==============================================" << endl;
	hash_map<int, vector<int> >::iterator hit;
	vector<int>::iterator vit;
	for (hit = outlist.begin(); hit != outlist.end(); hit++) {
		cout << hit->first << ": ";
		vector<int> vec = hit->second;
		for (vit = vec.begin(); vit != vec.end(); vit++)
			cout << *vit << " ";
		cout << "#" << endl;
	}
	cout << "In List for graph" << endl;
	for (hit = inlist.begin(); hit != inlist.end(); hit++) {
		cout << hit->first << ": ";
		vector<int> vec = hit->second;
		for (vit = vec.begin(); vit != vec.end(); vit++)
			cout << *vit << " ";
		cout << "#" << endl;
	}
	cout << "================================================" << endl;
}

void Graph::print_edges() {
	cout << "----Current Edge sets: ----" << endl;
	EdgeList el;
	for (int i = 0; i < num_vertices(); i++) {
		el = graph[i].outList;
		for(const auto & e_it:el){
			cout << i << "->" << e_it << endl;
		}
	}
	cout << "---------------------------" << endl;
}

const double Graph::tcs(const int vid) {
	return vl[vid].tcs;
}

void Graph::sortEdges() {
	GRA::iterator git;
	for (git = graph.begin(); git != graph.end(); git++) {
		sort(git->inList.begin(), git->inList.end());
		sort(git->outList.begin(), git->outList.end());
	}
}

vector<string>& Graph::split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while(getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> Graph::split(const string &s, char delim) {
	vector<string> elems;
	return split(s, delim, elems);
}

void Graph::remove_edge(int src, int trg) {
	EdgeList *succs = &graph[src].outList;
	auto f_trg = find(succs->begin(), succs->end(), trg);
	assert(f_trg != succs->end());
	succs->erase(f_trg);

	auto *preds = &graph[trg].inList;
	auto f_it = find(preds->begin(), preds->end(), src);
	assert(f_it != preds->end());
	preds->erase(f_it);
}
