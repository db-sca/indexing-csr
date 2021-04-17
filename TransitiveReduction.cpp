//
// Created by chaowyc on 3/4/2021.
//

#include "TransitiveReduction.h"

TransitiveReduction::TransitiveReduction(Graph &g) : vfg(g){
	lpm_tree = Graph(vfg.num_vertices());
	for (int i = 0; i < vfg.num_vertices(); ++i) {
		lpm_tree.addVertex(i);
	}
}

void TransitiveReduction::markCNRNN() {
	construct_lpm_tree();
	interval_labelling();
	vector<int> RNVec;
	set<int> NonRNSet;

	for(const auto & v : vfg.vertices()){
		if (is_reducible_node(v.id)){
			RNVec.push_back(v.id);
#ifdef TRDEBUG
			cout << v.id << " is a RN." << endl;
#endif
		} else {
			NonRNSet.insert(v.id);
#ifdef TRDEBUG
			cout << v.id << " is not a RN." << endl;
#endif
		}
	}

	sort(RNVec.begin(), RNVec.end(), [this](int a, int b){ return vfg[a].topo_id < vfg[b].topo_id; });

	set<int> RNSet(RNVec.begin(), RNVec.end());

	vector<int> RRNVec;
	set<int> RRNSet;

	set<int> NonRRNSet;
	vector<int> NonRRNVec;

	for(const auto & v : RNSet){
		if (is_removable_RN(vfg, v, RRNSet)){
			RRNSet.insert(v);
#ifdef TRDEBUG
			cout << v << " is a RRN." << endl;
#endif
		} else {
			NonRRNVec.emplace_back(v);
#ifdef TRDEBUG
			cout << v << " is not a RRN." << endl;
#endif
		}
	}

	find_CN_from_NonRRN(vfg, NonRRNVec);
	reduced_graph = Graph(vfg);
	for(const auto rrn : RRNSet){
		reduced_graph.remove_vertex(rrn);
	}
#ifdef TRDEBUG
	cout << "Edge-removed Graph " << endl;
	for(const auto & v : vfg.vertices()){
		for(const auto & succ : vfg.out_edges(v.id)){
			cout << v.id  << "->" << succ << endl;
		}
	}
	cout << "RRN-removed Graph " << endl;
	for(const auto & v : reduced_graph.vertices()){
		for(const auto & succ : reduced_graph.out_edges(v.id)){
			cout << v.id  << "->" << succ << endl;
		}
	}

#endif
}

void TransitiveReduction::gen_po_tree() {
	po_tree = Graph(reduced_graph.num_vertices() + 1);
	for (int i = 0; i <= reduced_graph.num_vertices() ; ++i) {
		po_tree.addVertex(i);
	}

	vector<int> rg_topo_sort;
	GraphUtil::topological_sort(reduced_graph, rg_topo_sort);
	for (int i = 0; i < rg_topo_sort.size(); i++) {
		if (reduced_graph[rg_topo_sort[i]].removed){
			po_tree[rg_topo_sort[i]].removed = true;
			continue;
		}
#ifdef TRDEBUG
		cout << "vid: " << rg_topo_sort[i] << " tpo: " << reduced_graph[rg_topo_sort[i]].topo_id << endl;
#endif
		estimate_N(rg_topo_sort[i]);
#ifdef TRDEBUG
		cout << "vid: " << rg_topo_sort[i] << " max_v: " << reduced_graph[rg_topo_sort[i]].max_v.first << " est: " << reduced_graph[rg_topo_sort[i]].estimated_num_desc << endl;
#endif
		po_tree.addEdge(reduced_graph[rg_topo_sort[i]].max_v.first, rg_topo_sort[i]);
	}
#ifdef TRDEBUG
	cout << "Po Tree" << endl;
	for(const auto & v : po_tree.vertices()){
		for(const auto & succ: po_tree.out_edges(v.id)){
			cout << v.id << "->" << succ << endl;
		}
	}
#endif
}

void TransitiveReduction::bottom_up() {
	// DT order
	vector<int> dt_order;
	vector<int> r_dt_order;

	flag = vector<int>(vfg.num_vertices(), vfg.num_vertices() + 1);
	edge = vector<int>(vfg.num_vertices(), -1);

	GraphUtil::topological_sort(po_tree, dt_order);
	GraphUtil::topological_sort_backwards(po_tree, r_dt_order);

	int r_dt_id = 0;
#ifdef TRDEBUG
	cout << "Reversed DT order of Po-tree: " << endl;
#endif
	for (const auto & v : r_dt_order) {
		if(po_tree[v].removed){
			continue;
		}
		po_tree[v].r_dt_id = r_dt_id;
#ifdef TRDEBUG
		cout << "vid:" << v << " rdt: " << r_dt_id << endl;
#endif
		r_dt_id++;
	}
#ifdef TRDEBUG
	cout << endl;

	cout << "DT order of Po-tree: " << endl;
#endif

	int dt_id = 0;
	for(int i = dt_order.size() - 1; i >= 0; i--){
		if (po_tree[dt_order[i]].removed){
			continue;
		}
		po_tree[dt_order[i]].dt_id = dt_id;
#ifdef TRDEBUG
		cout << "vid: " << dt_order[i] << " dt: " <<  dt_id << endl;
#endif
		dt_id++;
	}
#ifdef TRDEBUG
	cout << endl;
#endif
	EdgeList il = po_tree.out_edges(reduced_graph.num_vertices());
//	sort(il.begin(), il.end(), bind(rdt_small_cmp, std::placeholders::_1, std::placeholders::_2, ref(po_tree)));
	sort(il.begin(), il.end(), [this](int a, int b){ return po_tree[a].r_dt_id < po_tree[b].r_dt_id; });

	for(const auto & v : po_tree.out_edges(reduced_graph.num_vertices())){
		process_tree_child(v, reduced_graph.num_vertices());
	}
}

void TransitiveReduction::process_tree_child(int src, int trg) {
#ifdef TRDEBUG
	cout << "processing edge: " << src << "->" << trg << endl;
#endif
	delete_redundant_edge(src, trg);
	EdgeList il = po_tree.out_edges(src);
//	sort(il.begin(), il.end(), bind(rdt_small_cmp, std::placeholders::_1, std::placeholders::_2, ref(po_tree)));
	sort(il.begin(), il.end(), [this](int a, int b){ return po_tree[a].r_dt_id < po_tree[b].r_dt_id; });

	for(const auto & v : po_tree.out_edges(src)){
		process_tree_child(v, src);
	}
}

void TransitiveReduction::delete_redundant_edge(int src, int trg) {
	int succ = 0;
	EdgeList ol = reduced_graph.out_edges(src);
	for (int i = 0; i < ol.size(); ++i) {
		succ = ol[i];
		if (flag[succ] <= po_tree[trg].dt_id){
#ifdef TRDEBUG
			cout << "rm edge: " << src << "->" << succ << endl;
#endif
			reduced_graph.remove_edge(src, succ);
			vfg.remove_edge(src, succ);
		} else {
			flag[succ] = po_tree[src].dt_id;
			edge[succ] = src;
		}
	}
	if (trg == po_tree.num_vertices()){
		return;
	}
}

void TransitiveReduction::construct_lpm_tree() {

//	GraphUtil::mergeSCC(g, sccmap, reverse_topo_sort);
	gen_dfs_spanning_tree();

	GraphUtil::topo_leveler(vfg);
	GraphUtil::topological_sort(vfg, g_topo_sort);
//	GraphUtil::topological_sort(tr.lpm_tree, t_topo_sort);
	for (int i = 0; i < g_topo_sort.size(); ++i) {
		vfg[g_topo_sort[i]].topo_id = g_topo_sort.size() - i - 1;
		lpm_tree[g_topo_sort[i]].topo_id = g_topo_sort.size() - i - 1;
		lpm_tree[g_topo_sort[i]].top_level = vfg[g_topo_sort[i]].top_level;
	}

//	for(const auto & v : vfg.vertices()){
//		cout << "vid: " << v.id << " tp level: " << v.top_level << " tp order: " << v.topo_id << endl;
//	}

#ifdef TRDEBUG
	for (const auto & v : lpm_tree.vertices()) {
		for (const auto & u : lpm_tree.out_edges(v.id)) {
			cout << v.id << " -> " << u << endl;
		}
	}
#endif
}

void TransitiveReduction::linear_ER() {
//	vector<set<int>> P;
//	set<int> V;
//	for(const auto v : vfg.vertices()){
//		V.insert(v.id);
//	}
//	P.push_back(V);
//	divide_and_conquer(P, 0);
//	for(const auto & p : P){
//		for(const auto & v : p){
//			cout << v << " ";
//		}
//		cout << endl;
//	}
	vector<int> P(vfg.num_vertices(), 0);
	for (int i = 0; i < vfg.num_vertices(); ++i) {
		refine(P, i);
	}
#ifdef TRDEBUG
	for(int i = 0; i < vfg.num_vertices(); i++){
		cout << "vid: " << i << " P_i: " << P[i] << endl;
	}
#endif
	for(int i = 0; i < P.size(); i++){
		vid_mapper[P[i]].emplace_back(i);
	}
	ER_g = Graph(vfg);
	ER = vector<int>(vfg.num_vertices(), 0);

	for(const auto & item : vid_mapper){
		int vid = item.second.front();
		for(const auto & v : item.second){
			ER[v] = vid;
		}
		if (item.second.size() <= 1){
			continue;
		}
		for(int i = 1; i < item.second.size(); i++){
			ER_g.remove_vertex(item.second[i]);
		}
	}



#ifdef TRDEBUG
	for(int i = 0; i < ER.size(); i++){
		cout << "vid: " << i << " new id: " << ER[i] << endl;
	}

	cout << "ER g:" << endl;
	for(const auto & v : ER_g.vertices()){
		for(const auto & succ : ER_g.out_edges(v.id)){
			cout << v.id << "->" << succ << endl;
		}
	}

#endif
}

void TransitiveReduction::refine(vector<int> &P, int vid) {
	EdgeList ol = vfg.out_edges(vid);
	EdgeList il = vfg.in_edges(vid);
#ifdef TRDEBUG
//	cout << "For vid: " << vid << endl;
#endif
	map<int, vector<int>> father_mapper;
	map<int, vector<int>> child_mapper;

	for(const auto v : ol){
		child_mapper[P[v]].emplace_back(v);
#ifdef TRDEBUG
//		cout << "\t" << P[v] << " " << v << endl;
#endif
	}
	for(const auto v : il){
		father_mapper[P[v]].emplace_back(v);
#ifdef TRDEBUG
//		cout << "\t" << P[v] << " " << v << endl;
#endif
	}
	for(auto & v : father_mapper){
		p_id++;
		for(auto & i : v.second){
			P[i] = p_id;
		}
	}

	for(auto & v : child_mapper){
		p_id++;
		for(auto & i : v.second){
			P[i] = p_id;
		}
	}
}

void TransitiveReduction::divide_and_conquer(vector<set<int>> &P, int vid) {

	while (vid < vfg.num_vertices()){
		cout << vid << endl;
		int length = P.size();
		for(int i = 0; i < length; i++){
			vector<set<int>> results = divide(P[i], vid);
			for(const auto res : results){
				if (res.empty()){
					continue;
				}
				P.emplace_back(res);
			}
			if (P[i].empty()){
				P.erase(P.begin() + i);
			}
		}
		vid++;
	}
}

vector<set<int>> TransitiveReduction::divide(set<int> &P_i, int vid) {
	EdgeList ol = vfg.out_edges(vid);
	EdgeList il = vfg.in_edges(vid);
	unordered_set<int>ol_set(ol.begin(), ol.end());
	unordered_set<int>il_set(il.begin(), il.end());

	set<int> parents;
	set<int> children;

	vector<set<int>> partition;
	for (auto v : P_i) {
		if (il_set.count(v)){
			parents.insert(v);
		} else if (ol_set.count(v)){
			children.insert(v);
		}
	}
	for(const auto v : parents){
		P_i.erase(v);
	}
	for(const auto v : children){
		P_i.erase(v);
	}
//	cout << "partition results:" << endl;
//	cout << "children: " << endl;
//	for(const auto & v : children){
//		cout << v << " " << endl;
//	}
//	cout << endl;
//
//	cout << "father: " << endl;
//	for(const auto & v : parents){
//		cout << v << " " << endl;
//	}
//	cout << endl;
//
//	cout << "others: " << endl;
//	for(const auto & v : others){
//		cout << v << " " << endl;
//	}
//	cout << endl;

	partition.emplace_back(children);
	partition.emplace_back(parents);
	return partition;
}

void TransitiveReduction::estimate_N(int vid) {
	if (reduced_graph.out_edges(vid).empty()){
		reduced_graph[vid].estimated_num_desc = 0;
		reduced_graph[vid].max_v.first = reduced_graph.num_vertices();
		reduced_graph[vid].max_v.second = 0;
		return;
	}
	int size_of_tree_descendants = lpm_tree[vid].topo_post->back() - lpm_tree[vid].topo_pre->back();

	int argmax_v = 0;
	int max_v = reduced_graph.out_edges(vid).front();
	for(const auto & succ : reduced_graph.out_edges(vid)){
#ifdef TRDEBUG
		cout << "\tsucc: " << succ << endl;
#endif
		if (reduced_graph[succ].estimated_num_desc >= argmax_v){
			argmax_v = reduced_graph[succ].estimated_num_desc;
			max_v = succ;
		}
	}
//	assert(max_v != -1);
//	assert(argmax_v != -1);
	reduced_graph[vid].max_v.first = max_v;
	reduced_graph[vid].max_v.second = argmax_v;

	if (CN.count(vid)){
		if (size_of_tree_descendants > argmax_v){
			reduced_graph[vid].estimated_num_desc = size_of_tree_descendants;
		} else {
			reduced_graph[vid].estimated_num_desc = argmax_v;
		}
	} else {
		int sum_tree_desc = 0;
		for(const auto & succ : reduced_graph.out_edges(vid)){
			sum_tree_desc += reduced_graph[succ].estimated_num_desc;
		}
		reduced_graph[vid].estimated_num_desc = max(reduced_graph.out_degree(vid) + argmax_v, reduced_graph.out_degree(vid) + sum_tree_desc);
	}
}

void TransitiveReduction::interval_labelling() {
	vector<int> roots = lpm_tree.getRoots();
	for(auto & v: lpm_tree.vertices()){
		v.topo_pre = new vector<int>();
		v.topo_post = new vector<int>();
	}
	vector<bool> visited(lpm_tree.num_vertices(), false);
	for(const auto & root : roots){
		labelling_visit(lpm_tree, root, visited);
	}
}

int TransitiveReduction::labelling_visit(Graph &tree, int vid, vector<bool> &visited) {
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);
	EdgeList::iterator eit;
	int max_tp_order = -1;
	for (eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit]){
			max_tp_order = max(max_tp_order, labelling_visit(tree, *eit, visited));
		}else {
			max_tp_order = max(max_tp_order, tree[*eit].topo_post->back());
		}
	}
	max_tp_order = max(max_tp_order, tree[vid].topo_id);
	tree[vid].topo_pre->push_back(tree[vid].topo_id);
	tree[vid].topo_post->push_back(max_tp_order);
#ifdef TRDEBUG
	cout << "for id " << vid << " max tp in subtree: " << max_tp_order << " interval: [" << tree[vid].topo_id << ", " << max_tp_order << "]" << endl;
#endif
	return max_tp_order;
}

bool TransitiveReduction::is_reducible_node(int vid) {
	int last_rn_start = 0;
	int last_rn_end = 0;
	int min_tp_level = lpm_tree.num_vertices() + 1;

	EdgeList ol(vfg.out_edges(vid).begin(), vfg.out_edges(vid).end());

//	sort(ol.begin(), ol.end(), bind(small_cmp, std::placeholders::_1, std::placeholders::_2, ref(vfg)));
	sort(ol.begin(), ol.end(), [this](int a, int b){ return vfg[a].topo_id < vfg[b].topo_id; });

#ifdef TRDEBUG
	cout << endl;
	cout << "Test on " << vid << endl;
#endif
	int succ = 0;
	for (int i = 0; i < ol.size(); ++i) {
		succ = ol[i];
//	for(const auto & succ : ol){
		if (contain(lpm_tree[vid].topo_pre->back(), lpm_tree[vid].topo_post->back(), lpm_tree[succ].topo_pre->back(), lpm_tree[succ].topo_post->back())){
#ifdef TRDEBUG
			cout << "SUCC at " << succ << endl;
#endif
			if (min_tp_level > lpm_tree[succ].top_level){
				min_tp_level = lpm_tree[succ].top_level;
			}
			if (contain(last_rn_start, last_rn_end, lpm_tree[succ].topo_pre->back(), lpm_tree[succ].topo_post->back())){
#ifdef TRDEBUG
				cout << "remove: " << vid << "->" << succ << endl;
#endif
				vfg.remove_edge(vid, succ);
			} else {
#ifdef TRDEBUG
				cout << "mid vid: " << succ << endl;
#endif
				last_rn_start = lpm_tree[succ].topo_pre->back();
				last_rn_end = lpm_tree[succ].topo_post->back();
			}
		}
		else {
#ifdef TRDEBUG
			cout << "FAIL at " << succ << "[" << last_rn_start << ", " << last_rn_end << "]" << " [" << lpm_tree[succ].topo_pre->back() << ", " << lpm_tree[succ].topo_post->back() << "]" << endl;
			cout << "min tp: " << min_tp_level << endl;
#endif
			if (!contain(last_rn_start, last_rn_end, lpm_tree[succ].topo_pre->back(), lpm_tree[succ].topo_post->back())) {
				if (min_tp_level < lpm_tree[succ].top_level){
					return false;
				} else {
					last_rn_start = lpm_tree[succ].topo_pre->back();
					last_rn_end = lpm_tree[succ].topo_post->back();
					min_tp_level = lpm_tree[succ].top_level;
				}
			} else {
//				cout << "succ " << succ.id << " enters here 2." << endl;
//				cout << "remove: " << vid << "->" << succ.id << endl;
				vfg.remove_edge(vid, succ);
			}
		}
	}
	return true;
}

bool TransitiveReduction::is_reducible_node(int vid, vector<pair<int, int>> &remove_edges) {
	int last_rn_start = 0;
	int last_rn_end = 0;
	int min_tp_level = lpm_tree.num_vertices() + 1;

	EdgeList ol = vfg.out_edges(vid);

	sort(ol.begin(), ol.end(), bind(small_cmp, std::placeholders::_1, std::placeholders::_2, ref(vfg)));

//	cout << endl;
//	cout << "Test on " << vid << endl;
	for(const auto & succ : ol){

		if (contain(lpm_tree[vid].topo_pre->back(), lpm_tree[vid].topo_post->back(), lpm_tree[succ].topo_pre->back(), lpm_tree[succ].topo_post->back())){
//			cout << "SUCC at " << succ << endl;
			if (min_tp_level > lpm_tree[succ].top_level){
				min_tp_level = lpm_tree[succ].top_level;
			}

			if (!contain(last_rn_start, last_rn_end, lpm_tree[succ].topo_pre->back(), lpm_tree[succ].topo_post->back())){
				last_rn_start = lpm_tree[succ].topo_pre->back();
				last_rn_end = lpm_tree[succ].topo_post->back();
			} else {
//				cout << "succ " << succ << " enters here 1." << endl;
//				cout << "remove: " << vid << "->" << succ << endl;
				remove_edges.push_back(make_pair(vid, succ));
//				vfg.remove_edge(vid, succ.id);
			}
		} else {
//			cout << "FAIL at " << succ << "[" << last_rn_start << ", " << last_rn_end << "]" << " [" << lpm_tree[succ].topo_pre->back() << ", " << lpm_tree[succ].topo_post->back() << "]" << endl;
//			cout << "min tp: " << min_tp_level << endl;
			if (!contain(last_rn_start, last_rn_end, lpm_tree[succ].topo_pre->back(), lpm_tree[succ].topo_post->back())) {
				if (min_tp_level < lpm_tree[succ].top_level){
					return false;
				} else {
					last_rn_start = lpm_tree[succ].topo_pre->back();
					last_rn_end = lpm_tree[succ].topo_post->back();
					min_tp_level = lpm_tree[succ].top_level;
				}
			} else {
//				cout << "succ " << succ << " enters here 2." << endl;
//				cout << "remove: " << vid << "->" << succ << endl;
				remove_edges.push_back(make_pair(vid, succ));
//				vfg.remove_edge(vid, succ.id);
			}
		}
	}
	return true;
}

bool TransitiveReduction::is_removable_RN(Graph &g_ref, int vid, set<int> &RRN) {
	EdgeList inl = g_ref.in_edges(vid);
	if (inl.empty()){
		return true;
	}

	for(const auto & pred : inl){
		if (!RRN.count(pred)){
			return false;
		}
	}
	return true;
}

void TransitiveReduction::find_CN_from_NonRRN(Graph &g_ref, vector<int> &NonRRN) {
	for (int i = 0; i < g_topo_sort.size(); ++i) {
		if (vfg[g_topo_sort[i]].topo_id > vfg[g_topo_sort[i]].max_u.second){
			vfg[g_topo_sort[i]].max_u.second = vfg[g_topo_sort[i]].topo_id;
			vfg[g_topo_sort[i]].max_u.first = g_topo_sort[i];
		}

		for(const auto & pred : g_ref.in_edges(g_topo_sort[i])){
			if (vfg[g_topo_sort[i]].max_u.second > vfg[pred].max_u.second){
				vfg[pred].max_u.first = vfg[g_topo_sort[i]].max_u.first;
				vfg[pred].max_u.second = vfg[g_topo_sort[i]].max_u.second;
			}
//			vfg[pred].max_x_u = max(vfg[pred].max_x_u, vfg[g_topo_sort[i]].max_x_u);
#ifdef TRDEBUG
			cout << "curr: " << g_topo_sort[i] << " father: " << pred << " max: " << vfg[pred].max_u.second << " from: " << vfg[pred].max_u.first << endl;
#endif
		}
	}

#ifdef TRDEBUG
	for(const auto & v : g_ref.vertices()){
		cout << "vid : " << v.id << " max: " << vfg[v.id].max_u.second << " from: " << vfg[v.id].max_u.first << endl;
	}
#endif
//	sort(NonRRN.begin(), NonRRN.end(), bind(big_cmp, std::placeholders::_1, std::placeholders::_2, ref(vfg)));
	sort(NonRRN.begin(), NonRRN.end(), [this](int a, int b){ return vfg[a].topo_id > vfg[b].topo_id; });

	int max_u;
	for (const auto & v : NonRRN) {
		max_u = vfg[v].max_u.first;
		if (contain(lpm_tree[v].topo_pre->back(), lpm_tree[v].topo_post->back(), lpm_tree[max_u].topo_pre->back(), lpm_tree[max_u].topo_post->back())){
#ifdef TRDEBUG
			cout << v << " is a CN." << endl;
#endif
			CN.insert(v);
		}
	}
}

bool TransitiveReduction::contain(int src_start, int src_end, int trg_start, int trg_end){
	if (src_start > trg_start){
		return false;
	}

	if (src_end < trg_end){
		return false;
	}
	return true;
}

void TransitiveReduction::gen_dfs_spanning_tree() {
	vector<bool> visited(vfg.num_vertices(), false);
	vector<int> roots = vfg.getRoots();
	vector<int>::iterator sit;
	// depth-first-search whole graph
	for (sit = roots.begin(); sit != roots.end(); sit++)
		if (!visited[*sit])
			dfs((*sit), visited);
}

void TransitiveReduction::dfs(int vid, vector<bool> &visited) {
	visited[vid] = true;
	EdgeList el = vfg.out_edges(vid);
	EdgeList::iterator eit;
	int nextid = -1;
	// check whether all child nodes are visited
	for (eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit]) {
			nextid = *eit;
			lpm_tree.addEdge(vid, nextid);
			dfs(nextid, visited);
		}
	}
}

bool small_cmp(int v1, int v2, Graph &g){
	if (g[v1].topo_id <= g[v2].topo_id) {
		return true;
	} else {
		return false;
	}
}

bool rdt_small_cmp(int v1, int v2, Graph &g){
	if (g[v1].r_dt_id <= g[v2].r_dt_id) {
		return true;
	} else {
		return false;
	}
}

bool big_cmp(int v1, int v2, Graph &g){
	if (g[v1].topo_id >= g[v2].topo_id){
		return true;
	} else {
		return false;
	}
}
