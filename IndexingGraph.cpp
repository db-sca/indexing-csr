//
// Created by chaowyc on 12/3/2021.
//

#include "IndexingGraph.h"

IndexingGraph::IndexingGraph(VFG &vfg) : vfg_ref(vfg){
	cout << "# Partial O_Vertices: " << vfg.num_vertices() << " # O_Edge: " << vfg.num_edges() << endl;
	construct_partial_indexing_graph();
	cout << "# Partial N_Vertices: " << this->num_vertices() << " # N_Edge: " << this->num_edges() << endl;
	assert(this->num_vertices() == 2 * vfg.num_vertices());
	assert(this->num_edges() == 2 * vfg.num_edges() + vfg.num_vertices());
}

int IndexingGraph::idx(int o_vid) {

	return o_vid + vfg_ref.num_vertices();
}

void IndexingGraph::makeup_summary_edges() {
//	for (int i = 0; i < vfg.num_vertices(); ++i) {
//		EdgeList inlist = vfg.in_edges(i);
//		EdgeList outlist = vfg.out_edges(i);
//		addVertex(i);
//		vl[i].o_vid = i;
//		addVertex(idx(i));
//		vl[idx(i)].o_vid = i;
//
//		for(const auto & pred : inlist){
//			addEdge(pred, i);
//			addEdge(idx(pred), idx(i));
//			vl[pred].o_vid = pred;
//			vl[idx(pred)].o_vid = pred;
//			addEdge(pred, idx(pred));
//		}
//		for(const auto & succ : outlist){
//			addEdge(i, succ);
//			addEdge(idx(i), idx(succ));
//			vl[succ].o_vid = succ;
//			vl[idx(succ)].o_vid = succ;
//			addEdge(succ, idx(succ));
//		}
//	}
	int n_ce = 0;
	int n_re = 0;
	int n_se = 0;
	for(const auto & cs_idt: vfg_ref.callsite_map){
		auto *cs_obj = cs_idt.second;
		n_ce += cs_obj->call_edges.size();
		n_re += cs_obj->ret_edges.size();
		n_se += cs_obj->input2output.size();

		assert(cs_obj->ret_edges.size() == cs_obj->ret2output.size());
		assert(cs_obj->call_edges.size() == cs_obj->input2arg.size());

		for(const auto & ce : cs_obj->call_edges){
			addEdge(idx(ce.first), idx(ce.second));
		}
		for(const auto & re : cs_obj->ret_edges){
			addEdge(re.first, re.second);
		}
		for(const auto & io : cs_obj->input2output){
			addEdge(io.first, io.second);
			addEdge(idx(io.first), idx(io.second));
		}
	}
	cout << "Add # Call-Edge: " << n_ce << " # Ret-Edge: " << n_re << "# Sum-Edge: " << n_se << endl;
	assert(this->num_vertices() == 2 * vfg_ref.num_vertices());
//	cout << this->num_edges() << endl;
//	cout << 2 * vfg.num_edges() + vfg.num_vertices() + 2 * n_se + n_ce + n_re << endl;
	assert(this->num_edges() == (2 * vfg_ref.num_edges() + vfg_ref.num_vertices() + 2 * n_se - n_ce - n_re));
}

void IndexingGraph::construct_partial_indexing_graph() {

	for (int i = 0; i < vfg_ref.num_vertices(); ++i) {
		addVertex(i);
		vl[i].o_vid = i;
		addVertex(idx(i));
		vl[idx(i)].o_vid = i;
		addEdge(i, idx(i));
	}

	for (int i = 0; i < vfg_ref.num_vertices(); ++i) {
		EdgeList outlist = vfg_ref.out_edges(i);
		for(const auto & succ : outlist){
			addEdge(i, succ);
			addEdge(idx(i), idx(succ));
			vl[succ].o_vid = succ;
			vl[idx(succ)].o_vid = succ;
		}
	}
}
