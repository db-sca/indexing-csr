#ifndef _GRAPH_H
#define _GRAPH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <algorithm>
#include <utility>
#include <cmath>
#include <string>
#include <cassert>
#include "bit_vector.h"

//#define DEBUG
//#define DFSDEBUG
//#define IFDSDEBUG
//#define CSDEBUG


#if defined __GNUC__ || defined __APPLE__
#include <ext/hash_map>
#else
#include <hash_map>
#endif

#include <unordered_map>

using namespace std;
using namespace __gnu_cxx;

#define MAX_VAL 100000000
#define MIN_VAL -100000000

enum NodeType{
	NORMAL=0,
	INPUT,
	ARG,
	RET,
	OUTPUT
};

struct Vertex {
	int id;
	bool visited;
	int min_parent_level;
	bool fat;	// fat node
	int topo_id;	// topological order
	int top_level;	// topological level
	int path_id;	// path id
	int dfs_order;
	int pre_order;	
	int post_order;
	int first_visit; // for test
	int kind = NodeType::NORMAL;
	int func_id = -1;
	int o_vid = -1;
	bool removed = false;

	double tcs;
	int mingap;
	vector<int> *pre;
	vector<int> *post;
	vector<int> *middle;

	vector<int> *topo_pre;
	vector<int> *topo_post;
	pair<int, int> max_u = {-1, -1};
	int estimated_num_desc = 0;
	pair<int, int> max_v = {-1, -1};

	int dt_id = -1;
	int r_dt_id = -1;

	Vertex(int ID) : id(ID) {
		top_level = -1;
		visited = false;
	}
	Vertex(){
		top_level = -1;
		visited = false;
	};

};

typedef vector<int> EdgeList;	// edge list represented by vertex id list
typedef vector<Vertex> VertexList;	// vertices list (store real vertex property) indexing by id

struct In_OutList {
	EdgeList inList;
	EdgeList outList;
};
typedef vector<In_OutList> GRA;	// index graph

class Graph {

protected:
	VertexList vl;
	GRA graph;
	int n_vertices = 0;
	int n_edges = 0;
public:
		Graph();
		Graph(int);
		Graph(GRA&, VertexList&);
		Graph(istream&);
		Graph(const string& graphs_folder);
		int vsize;
		~Graph();

		virtual void readGraph(istream&);
		void writeGraph(ostream&);
		void printGraph();
		void addVertex(int);
		void remove_vertex(int);
		void remove_edge(int src, int trg);
		void addEdge(int, int);
		int num_vertices();
		int num_edges();
		VertexList& vertices();
		EdgeList& out_edges(int);
		EdgeList& in_edges(int);
		int out_degree(int);
		int in_degree(int);
		vector<int> getRoots();
		vector<int> get_leaves();
		bool hasEdge(int, int);
		Graph& operator=(const Graph&);
		Vertex& operator[](const int);
		
		void clear();
		void strTrimRight(string& str);

		Graph(hash_map<int,vector<int> >& inlist, hash_map<int,vector<int> >& outlist);
		void extract(hash_map<int,vector<int> >& inlist, hash_map<int,vector<int> >& outlist);
		void printMap(hash_map<int,vector<int> >& inlist, hash_map<int,vector<int> >& outlist);
		void print_edges();
		const double tcs(const int);
		void sortEdges();
		static vector<string> split(const string &s, char delim);
		static vector<string>& split(const string &s, char delim, vector<string> &elems);
};	

#endif
