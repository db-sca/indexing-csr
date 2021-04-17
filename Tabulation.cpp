//
// Created by chaowyc on 8/3/2021.
//

#include "Tabulation.h"
#include "progressbar.h"
//#define DEBUG

Tabulation::Tabulation(VFG &g, CallGraph &cg) : g(g), cg(cg){
	this->path_edge = new set<iEdge>();
	this->work_list = new vector<iEdge>();
	this->summary_edge = new set<iEdge>();
	this->reachable_call_edge = new set<iEdge>();
}

Tabulation::Tabulation(VFG &g, CallGraph &cg, bool is_backward): g(g), cg(cg) {
	if (is_backward){
		this->reachable_node = new set<int>;
		this->visited_node = new set<int>;
		this->node_stack = new vector<int>;
		this->reachable_node_relevant_to_demand = new vector<int>;
	}
	this ->path_edge = new set<iEdge>();
	this->work_list = new vector<iEdge>();
	this->summary_edge = new set<iEdge >();
	this->reachable_call_edge = new set<iEdge>();
}

// single src single sink forward_reach
//bool Tabulation::forward_reach(int vid, int sink) {
//	this->path_edge->clear();
//	this->work_list->clear();
//	this->summary_edge->clear();
//	this->reachable_call_edge->clear();
//#ifdef IFDSDEBUG
//	cout << "IFDS path: (" << vid << ", " << sink << ")" << endl;
//#endif
////#ifdef DEBUG
////	this->print_node_type();
////	this->print_cs_inputarg();
////#endif
//	// special case for arg type src
//	int src_kind = g[vid].kind;
//	if(src_kind == NodeType::ARG){
//		auto cs_idr = g.arg2csid.equal_range(vid);
//		for(auto it = cs_idr.first; it != cs_idr.second; it++){
//			int cs_id = it->second;
//			CallSite *cs_obj = g.callsite_map[cs_id];
//			for(const auto & input : cs_obj->inputs){
//				if (cs_obj->call_edges.count(iEdge(input, vid))){
//					this->path_edge->insert(iEdge(input, vid));
////					this->reachable_call_edge->insert(Edge(input, vid));
//				}
//			}
//		}
//	}
//	this->path_edge->insert(make_pair(vid, vid));
//	this->work_list->push_back(make_pair(vid, vid));
//#ifdef DEBUG
//	cout << "init WL: insert (" << vid << ", " << vid << ")" << endl;
//#endif
//	while(!this->work_list->empty()){
//		iEdge edge = this->work_list->back();
//		this->work_list->pop_back();
//#ifdef DEBUG
//		cout << "pop WL: pop (" << edge.first << ", " << edge.second << ")" << endl;
//#endif
//		int main = edge.first;
//		int sid = edge.second;
//		int n_type = g[sid].kind;
//#ifdef IFDSDEBUG
//		cout << sid << "(" << n_type << ")" << endl;
//#endif
//		if(sid == sink){
//#ifdef DEBUG
//			cout << vid << " reaches " << sink << " !!!!!!!!!!!!" << endl;
//#endif
//#ifdef IFDSDEBUG
//			cout << "reach " << sink << endl;
//#endif
//			return true;
//		}
//#ifdef DEBUG
//		cout << "type: " << sid << " is " << n_type << endl;
//#endif
//		if (n_type == NodeType::INPUT){
//			auto cs_idr = g.input2csid.equal_range(sid);
//
//			for(auto it = cs_idr.first; it != cs_idr.second; it++){
//				int cs_id = it->second;
//				CallSite *cs_obj = g.callsite_map[cs_id];
//				this->propagate(cs_obj->input2arg[sid], cs_obj->input2arg[sid]);
//				this->path_edge->insert(iEdge(sid, cs_obj->input2arg[sid]));
//				this->reachable_call_edge->insert(iEdge(sid, cs_obj->input2arg[sid]));
//#ifdef DEBUG
//				cout << "insert reachable ce: (" << sid << ", " << cs_obj->input2arg[sid] << "), size: " << this->reachable_call_edge->size() << endl;
//#endif
//#ifdef DEBUG
//				cout << "insert c-e WL at cs " << cs_id << ": insert (" << sid << ", " << cs_obj->input2arg[sid] << ")" << endl;
//#endif
//			}
//
//			for(auto it = cs_idr.first; it != cs_idr.second; it++){
//				int cs_id = it->second;
//				CallSite *cs_obj = g.callsite_map[cs_id];
//				for(int output : cs_obj->outputs){
//						if(g.hasEdge(sid, output) || this->summary_edge->count(iEdge(sid, output))){
//						this->propagate(main, output);
//					}
////					else if(this->summary_edge->count(Edge(sid, output)) > 0){
////						this->propagate(main, output);
////					}
//				}
//			}
//		} else if(n_type == NodeType::RET) {
//#ifdef DEBUG
//			cout << "--- process ret ---" << endl;
//#endif
//			bool callee2caller = false;
//			if (this->reachable_call_edge->empty()){
//				callee2caller = true;
//			}
//#ifdef DEBUG
//			cout << "reachable ce status: " << callee2caller << endl;
//#endif
//			auto cs_idr = g.ret2csid.equal_range(sid);
//			for(auto it = cs_idr.first; it != cs_idr.second; it++){
//				int cs_id = it->second;
//				CallSite *cs_obj = g.callsite_map[cs_id];
//				int output = cs_obj->ret2output[sid];
//#ifdef DEBUG
//				cout << "found output at cs: " << cs_obj->id << " " << output << endl;
//#endif
//				if(callee2caller && this->reachable_call_edge->empty()){
//#ifdef DEBUG
//					cout << "reachable ce EMPTY!" << endl;
//					cout << "propagates r-e: (" << sid << ", " << output << ")" << endl;
//#endif
//					this->propagate(sid, output);
//				}
//				for(const auto & igt : cs_obj->input2arg) {
//					if (cs_obj->call_edges.count(iEdge(igt.first, igt.second))) {
//#ifdef DEBUG
//						cout << "existing matched c-e: (" << igt.first << ", " << igt.second << ")" << endl;
//#endif
//						if (cs_obj->ret_edges.count(iEdge(sid, output))) {
//#ifdef DEBUG
//							cout << "r-e: (" << sid << ", " << output << ")" << endl;
//#endif
////							assert(this->path_edge->count(Edge(igt.second, sid)));
//							if (this->path_edge->count(iEdge(igt.second, sid)) && !this->summary_edge->count(iEdge(igt.first, output))) {
//								this->summary_edge->insert(iEdge(igt.first, output));
//#ifdef DEBUG
//								cout << "insert sum-edge: (" << igt.first << ", " << output << ")" << endl;
//#endif
//								if(this->path_edge->count(iEdge (igt.first, igt.second))){
//									if (this->reachable_call_edge->count(iEdge(igt.first, igt.second))){
//										this->reachable_call_edge->erase(iEdge(igt.first, igt.second));
//#ifdef DEBUG
//										cout << "remove reachable ce: (" << igt.first << ", " << igt.second << ")" << endl;
//										cout << "reachable CE size: " << this->reachable_call_edge->size() << endl;
//#endif
//									}
////									this->propagate(igt.first, output);
//									this->propagate(sid, output);
//
//#ifdef DEBUG
//									cout << "propagate sum-edge: (" << igt.first << ", " << output << ")" << endl;
//#endif
//								}
////								this->propagate(sid, output);  // for test
//								int func_id = g[igt.first].func_id;
//#ifdef DEBUG
//								cout << "func id: "<< func_id << endl;
//#endif
//								if(this->cg.roots->count(func_id)){
//
//									this->propagate(igt.first, output);
//#ifdef DEBUG
//									cout << "root!" << endl;
//									cout << "propagate: (" << igt.first << ", " << output << ")" << endl;
//#endif
//								}
//								if(g.function_map.count(func_id)){
//									auto *func_obj = g.function_map[func_id];
//									for (const auto &ait : func_obj->args) {
//#ifdef DEBUG
//										cout << "test existing of arg-input in PE: (" << ait << ", " << igt.first << ")" << endl;
//#endif
//										if (this->path_edge->count(iEdge(ait, igt.first)) > 0) {
//#ifdef DEBUG
//											cout << "existing arg-input: (" << ait << ", " << igt.first << ")" << endl;
//#endif
//											this->propagate(ait, output);
//#ifdef DEBUG
//											cout << "propagate: (" << ait << ", " << output << ")" << endl;
//#endif
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		} else {
//			EdgeList el = g.out_edges(sid);
//			for(auto it = el.rbegin(); it != el.rend(); it++){
//#ifdef DEBUG
//				cout << "propagate: (" << main << ", " << *it << ")" << endl;
//#endif
//				this->propagate(main, *it);
//			}
//
////			for(const auto & it : el){
////#ifdef DEBUG
////				cout << "propagate: (" << main << ", " << it << ")" << endl;
////#endif
////				this->propagate(main, it);
////			}
//		}
//#ifdef DEBUG
//		this->print_path_edge();
//		this->print_summary_edge();
//#endif
//	}
//#ifdef IFDSDEBUG
//	cout << endl;
//#endif
//	return false;
//}

bool Tabulation::forward_reach_recursive(int src, int sink) {
	clear_summary();
	vector<iEdge> self_work_list;
#ifdef IFDSDEBUG
	cout << "IFDS path: " << endl;
#endif
	self_work_list.emplace_back(src, src);
	set<int> self_visited_node;
	while (!self_work_list.empty()){
		iEdge e = self_work_list.back();
		self_work_list.pop_back();
		int main = e.first;
		int sid = e.second;
		int sid_type = this->g[sid].kind;
#ifdef IFDSDEBUG
		cout << sid << "(" << sid_type  << ")" << endl;
#endif
		if(self_visited_node.count(sid)){
			continue;
		}
		self_visited_node.insert(sid);

		if(sid == sink){
#ifdef IFDSDEBUG
			cout << src << " reach " << sink << endl;
#endif
			return true;
		}

		int curr_func_id = this->g[sid].func_id;

		if (sid_type == NodeType::INPUT){
			auto cs_idr = g.input2csid.equal_range(sid);
			for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
#ifdef SUMDEBUG
				cout << "input-cs: " << cs_id->second << endl;
#endif
				auto * cs_obj = g.callsite_map[cs_id->second];
				if (!cs_obj->summary_edge.count(sid)){
					for(const auto & ce: cs_obj->call_edges){
						if(sid == ce.first){
#ifdef IFDSDEBUG
							cout << "main enter ce, build se: (" << ce.first << ", " << ce.second  << ")" << "cs: " << cs_obj->id << " callee func: " << cs_obj->callee_func_id << endl;
#endif
							if (build_summary(ce.second, sink)){
								return true;
							}
						}
					}
				}
				auto se_r = cs_obj->summary_edge.equal_range(sid);
				for(auto se = se_r.first; se != se_r.second; se ++){
#ifdef IFDSDEBUG
					cout << "main use sum-edge at cs " << cs_obj->id << "(" << se->first << ", " << se->second << ")" << endl;
#endif
//					cout << "use se: (" << se->first << ", " << se->second << endl;
					self_work_list.emplace_back(main, se->second);
				}
			}
			EdgeList succs = g.out_edges(sid);
			for(const auto & succ : succs){
				if (g[succ].kind == NodeType::ARG){
					continue;
				}
				self_work_list.emplace_back(main, succ);
			}
		} else {
			EdgeList succs = g.out_edges(sid);
			for(const auto & succ : succs){
				self_work_list.emplace_back(main, succ);
#ifdef SUMDEBUG
				cout << "normal propagate: (" << main << ", " << succ << ")" << endl;
#endif
			}
		}
	}
	return false;
}

bool Tabulation::build_summary(int src, int sink){

	int curr_func_id = this->g[src].func_id;
	auto *func_obj = this->g.function_map[curr_func_id];
	const auto &rets = func_obj->rets;

	set<int> self_visited_node;
	vector<iEdge> self_work_list;
	set<iEdge> self_summary_edge;

#ifdef IFDSDEBUG
	cout << "current arg: " << src << endl;
#endif
	self_work_list.emplace_back(src, src);
	while(!self_work_list.empty()){
		iEdge e = self_work_list.back();
		self_work_list.pop_back();
#ifdef SUMDEBUG
		cout << "pop WL: " << "(" << e.first << ", " << e.second << ")" << endl;
#endif
		int main = e.first;
		int vid = e.second;
		if (self_visited_node.count(vid)){
			continue;
		}

		self_visited_node.insert(vid);
		if (vid == sink){
#ifdef IFDSDEBUG
			cout << "reach " << sink << endl;
#endif
			return true;
		}
		int vid_type = g[vid].kind;
		int vid_func_id = g[vid].func_id;
#ifdef IFDSDEBUG
		cout << vid << "(" << vid_type  << ")" << endl;
#endif

#ifdef SUMDEBUG
		cout << "start: " << vid << " type: " << vid_type << " func_id: " << curr_func_id << endl;
#endif

		if(vid_type == NodeType::INPUT){
			auto cs_idr = g.input2csid.equal_range(vid);
			for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
#ifdef SUMDEBUG
				cout << "input-cs: " << cs_id->second << endl;
#endif
				auto * cs_obj = g.callsite_map[cs_id->second];
				if (!cs_obj->summary_edge.count(vid)){
					for(const auto & ce: cs_obj->call_edges){
						if(vid == ce.first){
#ifdef IFDSDEBUG
							cout << "enter ce, build se: (" << ce.first << ", " << ce.second  << ")" << "cs: " << cs_obj->id << " callee func: " << cs_obj->callee_func_id << endl;
#endif
							if (build_summary(ce.second, sink)){
								return true;
							}
						}
					}
				}

				auto se_r = cs_obj->summary_edge.equal_range(vid);
				for(auto se = se_r.first; se != se_r.second; se ++){
#ifdef IFDSDEBUG
					cout << "use sum-edge at cs " << cs_obj->id << "(" << se->first << ", " << se->second << ")" << endl;
#endif
//					cout << "use se at cs: " << cs_obj->id << " (" << se->first << ", " << se->second << ")" << endl;
					self_work_list.emplace_back(main, se->second);
				}
			}
			EdgeList succs = g.out_edges(vid);
			for(const auto & succ : succs){
				if (g[succ].kind == NodeType::ARG){
					continue;
				}
				self_work_list.emplace_back(main, succ);
			}
		} else if (vid_type == NodeType::RET){
			if (vid_func_id != curr_func_id){
				cout << "ret func_id not equal: (" << vid << "." << vid_func_id << " , " << main << "." << curr_func_id << endl;
			}
			assert(vid_func_id == curr_func_id);
//			cout << "found se: (" << src << ", " << vid << ")" << endl;
			self_summary_edge.insert(iEdge(src, vid));
#ifdef IFDSDEBUG
			cout << "reach ret:" << vid << endl;
			cout << "insert arg-ret " << "(" << src << ", " << vid << ")" << " in graph. " << endl;
#endif
		}  else {
			EdgeList succs = g.out_edges(vid);
			for(const auto & succ : succs){
				if (this->g[succ].func_id != curr_func_id){
					cout << "func_id not equal: (" << vid << "." << vid_func_id << "." << curr_func_id << " , " << succ << "." << g[succ].func_id << endl;
					cout << "succ type: " << g[succ].kind << " vid type: " << vid_type << endl;
				}
				assert(this->g[succ].func_id == curr_func_id);
				self_work_list.emplace_back(main, succ);
#ifdef SUMDEBUG
				cout << "normal propagate: (" << main << ", " << succ << ")" << endl;
#endif
			}
		}
#ifdef SUMDEBUG
		print_path_edge();
		print_summary_edge();
#endif
	}
	// postprocessing: add input-output edge to callers
	auto cs_idr = g.arg2csid.equal_range(src);
	for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
		auto *cs_obj = g.callsite_map[cs_id->second];
		for(const auto & ce : cs_obj->call_edges){
			for(const auto & re : cs_obj->ret_edges){
				if (self_summary_edge.count(iEdge(ce.second, re.first))){
//					cout << "add i-o at cs :"  << cs_obj->id << " (" << ce.first << ", " << re.second << ")" << endl;
					cs_obj->summary_edge.insert(iEdge(ce.first, re.second));
#ifdef IFDSDEBUG
					cout << "At cs " << cs_obj->id << "callee func: " << cs_obj->callee_func_id << " insert in-out: " << "(" << ce.first << ", " << re.second << ")" << endl;
#endif
				}
			}
		}
	}
	return false;
}
//bool Tabulation::is_member_of_solution(int sid) {
//	int res = this->backward_DFS(sid);
//#ifdef DEBUG
//	cout << " Final result: " << res << endl;
//#endif
//	if (res == -1){
//		return false;
//	} else {
//		update_reachable_node(res);
//		return true;
//	}
//}

//int Tabulation::backward_DFS(int sid) {
//	this->reachable_node_relevant_to_demand->clear();
//	this->node_stack->clear();
//	this->visit(sid);
//
//#ifdef DEBUG
//	cout << "sink: " << sid << endl;
//#endif
//	while (!this->node_stack->empty()) {
//		int vid = this->node_stack->back();
//		this->node_stack->pop_back();
//#ifdef DEBUG
//		cout << "pop SK: " << vid << endl;
//#endif
//		int vid_type = this->g[vid].kind;
//		if (vid_type == NodeType::OUTPUT) {
//			auto cs_idr = this->g.output2csid.equal_range(vid);
//			for (auto cs_id_it = cs_idr.first; cs_id_it != cs_idr.second; cs_id_it++) {
//				auto *cs_obj = this->g.callsite_map[cs_id_it->second];
//				for (const auto &input_it : cs_obj->inputs) {
//					this->work_list->clear();
//					for (const auto &ret_it : cs_obj->rets) {
//						if (cs_obj->ret_edges.count(iEdge(ret_it, vid)) > 0) {
//							this->propagate(ret_it, ret_it);
//						}
//					}
//					this->backward_reach();
//#ifdef DEBUG
//					this->print_path_edge();
//					this->print_summary_edge();
//#endif
//					if (this->g.hasEdge(input_it, vid) || this->summary_edge->count(iEdge(input_it, vid))) {
//						this->visit(input_it);
//					}
//				}
//			}
//		} else if (vid_type == NodeType::ARG) {
//			auto cs_idr = this->g.arg2csid.equal_range(vid);
//			for (auto cs_id_it = cs_idr.first; cs_id_it != cs_idr.second; cs_id_it++) {
//				auto *cs_obj = this->g.callsite_map[cs_id_it->second];
//				for (const auto &input_it : cs_obj->inputs) {
//					this->visit(input_it);
////					if (cs_obj->call_edges.count(Edge(input_it, vid))) {
////						if (!this->visited_node->count(input_it)) {
////							this->node_stack->push_back(input_it);
////						}
////					}
//				}
//			}
//		} else {
//			EdgeList el = g.in_edges(vid);
//			for (const auto &it : el) {
//#ifdef DEBUG
//				cout << "propagate: (" << it << ", " << vid << ")" << endl;
//#endif
//				this->visit(it);
//			}
//#ifdef DEBUG
//			this->print_reachable_node();
//#endif
//		}
//	}
//	this->update_reachable_node();
//	if(this->reachable_node->count(sid)){
//#ifdef DEBUG
//		cout << sid << " in reachable_node" << endl;
//#endif
//		return true;
//	} else {
//#ifdef DEBUG
//		cout << sid << " not in reachable_node" << endl;
//#endif
//		return false;
//	}
//}


//void Tabulation::backward_reach() {
//	while(!this->work_list->empty()){
//		iEdge e =this->work_list->back();
//		this->work_list->pop_back();
//		int sid = e.first;
//		int tid = e.second;
//		int sid_type = this->g[sid].kind;
//		if(sid_type == NodeType::OUTPUT){
//			auto cs_idr = this->g.output2csid.equal_range(sid);
//			for(auto cs_id_it = cs_idr.first; cs_id_it != cs_idr.second; cs_id_it++){
//				auto *cs_obj = this->g.callsite_map[cs_id_it->second];
//				for(const auto & input_it : cs_obj->inputs){
//					for(const auto & ret_it : cs_obj->rets){
//						if(cs_obj->ret_edges.count(iEdge(ret_it, sid)) > 0){
//							this->propagate(ret_it, ret_it);
//						}
//					}
//					if(this->g.hasEdge(input_it, sid) || this->summary_edge->count(iEdge(input_it, sid))){
//						this->propagate(input_it, tid);
//					}
//				}
//			}
//		} else if(sid_type == NodeType::ARG){
//			auto cs_idr = this->g.arg2csid.equal_range(sid);
//			for(auto cs_id_it = cs_idr.first; cs_id_it != cs_idr.second; cs_id_it++){
//				auto *cs_obj = this->g.callsite_map[cs_id_it->second];
//				for(const auto & input_it : cs_obj->inputs){
//					for(const auto & output_it : cs_obj->outputs){
//						if(this->g.hasEdge(input_it, sid) && this->g.hasEdge(tid, output_it)){
//							if(!this->summary_edge->count(iEdge(input_it, output_it))){
//								this->summary_edge->insert(iEdge(input_it, output_it));
//								int func_id = cs_obj->callee_func_id;
//								if(this->g.function_map.count(func_id)){
//									auto *func_obj = this->g.function_map[func_id];
//									for(const auto & ret_it : func_obj->rets){
//										if(this->path_edge->count(iEdge(output_it, ret_it))){
//											this->propagate(input_it, ret_it);
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		} else {
//			EdgeList el = g.in_edges(sid);
//			for(const auto & it : el){
//#ifdef DEBUG
//				cout << "propagate: (" << it << ", " << sid << ")" << endl;
//#endif
//				this->propagate(it, tid);
//			}
//		}
//	}
//}

void Tabulation::update_reachable_node(int vid){
	vector<int> node_work_list;
	node_work_list.push_back(vid);
	while(!node_work_list.empty()){
		int node = node_work_list.back();
		node_work_list.pop_back();
		this->reachable_node->insert(node);
		this->visited_node->erase(node);
		EdgeList el = g.out_edges(node);
		for(const auto & it : el){
			bool is_ret_edge = false;
			for(const auto & cs_it : this->g.callsite_map){
				auto *cs_obj = cs_it.second;
				if (cs_obj->ret_edges.count(iEdge(node, it))){
					is_ret_edge = true;
					break;
				}
			}
			if (is_ret_edge){
				continue;
			} else {
				if (this->visited_node->count(it) && !this->reachable_node->count(it)){
					node_work_list.push_back(it);
				}
			}
		}
	}
}

//void Tabulation::update_reachable_node() {
//	while(!this->reachable_node_relevant_to_demand->empty()){
//		int node = this->reachable_node_relevant_to_demand->back();
//		this->reachable_node_relevant_to_demand->pop_back();
//		EdgeList el = g.out_edges(node);
//		for(const auto & it : el){
//			bool is_ret_edge = false;
//			for(const auto & cs_it : this->g.callsite_map){
//				auto *cs_obj = cs_it.second;
//				if (cs_obj->ret_edges.count(iEdge(node, it))){
//					is_ret_edge = true;
//#ifdef DEBUG
//					cout << "(" << node << ", " << it << ") is a r-e" << endl;
//#endif
//					break;
//				}
//			}
//			if (is_ret_edge){
//				continue;
//			} else {
//				if (this->visited_node->count(it) && !this->reachable_node->count(it)){
//					this->reachable_node->insert(it);
//					this->reachable_node_relevant_to_demand->push_back(it);
//				}
//			}
//		}
//	}
//}

//void Tabulation::visit(int sid) {
//	if(this->reachable_node->count(sid)){
//		this->reachable_node_relevant_to_demand->push_back(sid);
//	} else if (!this->visited_node->count(sid)){
//		this->visited_node->insert(sid);
//		this->node_stack->push_back(sid);
//	}
//}

void Tabulation::propagate(int sid, int tid) {
	iEdge e(sid, tid);
	if (!this->path_edge->count(e)){
		this->path_edge->insert(e);
		this->work_list->push_back(e);
	}
}

void Tabulation::print_path_edge() {
	cout << "----current path_edge: ----" << endl;
	for(const auto & it : *this->path_edge){
		cout << "(" << it.first << ", " << it.second << ")" << endl;
	}
	cout << "---------------------------" << endl;
}

void Tabulation::print_summary_edge() {
	cout << "----current summary_edge: ----" << endl;
	for(const auto & it : *this->summary_edge){
		cout << "(" << it.first << ", " << it.second << ")" << endl;
	}
	cout << "---------------------------" << endl;
}

void Tabulation::print_cs_inputarg() {
	cout << "==== Input Arg ====" << endl;
	for(auto iit = g.input2csid.begin(); iit != g.input2csid.end(); iit++){
		int cs_id = iit->second;
		CallSite *cs_obj = g.callsite_map[cs_id];
		for(auto cit = cs_obj->input2arg.begin(); cit != cs_obj->input2arg.end(); cit++){
			cout << "(" << cit->first << ", " << cit->second << ")" << endl;
		}
	}
	cout << "===================" << endl;
}

void Tabulation::print_node_type() {
	for(int i = 0; i < g.num_vertices(); i++){
		cout << i << ": " << g[i].kind << endl;
	}
}

void Tabulation::process_leaves(int func_id){
	vector<iEdge> self_work_list;
	set<int> self_visited_node;
	set<iEdge> self_summary_edge;

	auto *func_obj = this->g.function_map[func_id];
	auto &sources = func_obj->args;
	auto &sinks = func_obj->rets;
	for(const auto & src : sources){
		self_visited_node.clear();
		self_summary_edge.clear();
		self_work_list.clear();

		self_work_list.emplace_back(src, src);
#ifdef SUMDEBUG
		cout << "current src: " << src << endl;
#endif
		while (!self_work_list.empty()){
			iEdge e = self_work_list.back();
			self_work_list.pop_back();
#ifdef SUMDEBUG
			cout << "Pop WL:" << "(" << e.first << ", " << e.second << ")" << endl;
#endif
			int main = e.first;
			int vid = e.second;
			int vid_type = g[vid].kind;
#ifdef SUMDEBUG
			cout << "start at: " << vid << ", type: " << vid_type << endl;
#endif
			if (self_visited_node.count(vid)){
#ifdef SUMDEBUG
				cout << "ALREADY VISITED " << vid << endl;
#endif
				continue;
			}
			self_visited_node.insert(vid);

			if (vid_type == NodeType::RET){
				self_summary_edge.insert(iEdge(src, vid));
#ifdef SUMDEBUG
				cout << "reach ret:" << vid << endl;
				cout << "insert arg-ret " << "(" << src << ", " << vid << ")" << " in graph. " << endl;
#endif
			} else {
				EdgeList succs = g.out_edges(vid);
				for(const auto & succ : succs){
					self_work_list.emplace_back(iEdge(main, succ));
#ifdef SUMDEBUG
					cout << "propagate: (" << main << ", " << succ << ")" << endl;
#endif
				}
			}
#ifdef SUMDEBUG
			print_path_edge();
			print_summary_edge();
#endif
		}
		// postprocessing: add input-output edge to callers
		auto cs_idr = g.arg2csid.equal_range(src);
		for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
			auto *cs_obj = g.callsite_map[cs_id->second];
			for(const auto & ce : cs_obj->call_edges){
				for(const auto & re : cs_obj->ret_edges){
					if (self_summary_edge.count(iEdge(ce.second, re.first))){
						cs_obj->input2output.insert(iEdge(ce.first, re.second));
#ifdef SUMDEBUG
						cout << "At cs " << cs_obj->id << " insert in-out: " << "(" << ce.first << ", " << re.second << ")" << endl;
#endif
					}
				}
			}
		}
	}
}

void Tabulation::process_nonleaves(int func_id) {
//	cout << "************************** process non-leaf func: " << func_id << " ******************************" << endl;
#ifdef SUMDEBUG
	cout << "************************** process non-leaf func: " << func_id << " ******************************" << endl;
#endif
	vector<iEdge> self_work_list;
	set<int> self_visited_node;
	set<iEdge> self_summary_edge;

	auto *func_obj = g.function_map[func_id];

	auto &sources = func_obj->args;
	auto &sinks = func_obj->rets;
	assert(func_id == func_obj->id);
	int curr_func_id = func_id;
	for(const auto & src : sources){
#ifdef SUMDEBUG
		cout << "current src: " << src << endl;
#endif
		self_work_list.clear();
		self_summary_edge.clear();
		self_visited_node.clear();
		self_work_list.emplace_back(iEdge(src, src));
		while(!self_work_list.empty()){
			iEdge e = self_work_list.back();
			self_work_list.pop_back();
#ifdef SUMDEBUG
			cout << "pop WL: " << "(" << e.first << ", " << e.second << ")" << endl;
#endif
			int main = e.first;
			int vid = e.second;
			int vid_type = g[vid].kind;
			int vid_func_id = g[vid].func_id;

#ifdef SUMDEBUG
			cout << "start: " << vid << " type: " << vid_type << " func_id: " << curr_func_id << endl;
#endif
			if(self_visited_node.count(vid)){
#ifdef SUMDEBUG
				cout << "ALREADY VISITED: " << vid << endl;
#endif
				continue;
			}
			self_visited_node.insert(vid);
			if(vid_type == NodeType::INPUT){
				auto cs_idr = g.input2csid.equal_range(vid);
				for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
#ifdef SUMDEBUG
					cout << "input-cs: " << cs_id->second << endl;
#endif
					auto * cs_obj = g.callsite_map[cs_id->second];
					for(const auto & se : cs_obj->input2output){
#ifdef SUMDEBUG
						cout << "sum-edge at cs " << cs_obj->id << "(" << se.first << ", " << se.second << ")" << endl;
#endif
						if (vid == se.first){
#ifdef SUMDEBUG
							cout << "Found sum-edge: " << "(" << se.first << ", " << se.second << ")" << endl;
#endif
							self_work_list.emplace_back(iEdge(main, se.second));
						}
					}
				}
				EdgeList succs = g.out_edges(vid);
				for(const auto & succ : succs){
					if (g[succ].kind == NodeType::ARG){
						continue;
					}
					self_work_list.emplace_back(iEdge(main, succ));
#ifdef SUMDEBUG
					cout << "arg propagate: (" << main << ", " << succ << ")" << endl;
#endif
				}
			} else if (vid_type == NodeType::RET){
				if (vid_func_id != curr_func_id){
					cout << "ret func_id not equal: (" << vid << "." << vid_func_id << " , " << main << "." << curr_func_id << ")" << endl;
					cout << "vid type: " << vid_type << " main type: " << g[main].kind << endl;
				}
				assert(vid_func_id == curr_func_id);
				self_summary_edge.insert(iEdge(src, vid));
#ifdef SUMDEBUG
				cout << "reach ret:" << vid << endl;
				cout << "insert arg-ret " << "(" << src << ", " << vid << ")" << " in graph. " << endl;
#endif
			} else {
				EdgeList succs = g.out_edges(vid);
				for(const auto & succ : succs){
					if (this->g[succ].func_id != curr_func_id){
						cout << "func_id not equal: " << vid << "." << vid_func_id << "." << curr_func_id << " " << succ << "." << g[succ].func_id << endl;
						cout << " vid type: " << vid_type << " succ type: " << g[succ].kind << endl;
					}
					assert(this->g[succ].func_id == curr_func_id);

					self_work_list.emplace_back(iEdge(main, succ));
#ifdef SUMDEBUG
					cout << "normal propagate: (" << main << ", " << succ << ")" << endl;
#endif
				}
			}
		}
		// postprocessing: add input-output edge to callers
		auto cs_idr = g.arg2csid.equal_range(src);
		for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
			auto *cs_obj = g.callsite_map[cs_id->second];
			for(const auto & ce : cs_obj->call_edges){
				for(const auto & re : cs_obj->ret_edges){
					if (self_summary_edge.count(iEdge(ce.second, re.first))){
						cs_obj->input2output.insert(iEdge(ce.first, re.second));
#ifdef SUMDEBUG
						cout << "At cs " << cs_obj->id << " insert in-out: " << "(" << ce.first << ", " << re.second << ")" << endl;
#endif
					}
				}
			}
		}
	}
}

void Tabulation::build_full_summary(bool verbose= false) {
	CallGraph *cg_copy = new CallGraph(this->cg);
	auto leaves = cg_copy->get_leaves();
#ifdef DEBUG
		cout << "leaves are: ";
		for(const auto & it : leaves){
			cout << it << " ";
		}
		cout << endl;
#endif
	for(const auto & leaf : leaves){
#ifdef SUMDEBUG
		cout << "process leaf function: " << leaf << endl;
#endif
		this->process_leaves(leaf);
		cg_copy->remove_vertex(leaf);
	}

#ifdef SUMDEBUG
	cout << "# Vertices: " << cg_copy->num_vertices() << " # Edges: " << cg_copy->num_edges() <<  endl;
#endif
	progressbar bar(cg_copy->num_vertices());
	while (cg_copy->num_vertices() > 0){
#ifdef SUMDEBUG
		cg_copy->print_edges();
#endif
		auto leaves = cg_copy->get_leaves();
#ifdef DEBUG
		cout << "leaves are: ";
		for(const auto & it : leaves){
			cout << it << " ";
		}
		cout << endl;
#endif
		for(const auto & leaf : leaves){
			// process non-leaf
			process_nonleaves(leaf);
			cg_copy->remove_vertex(leaf);
#ifdef SUMDEBUG
			cout << "remove leaf: " << leaf << endl;
#endif
			bar.update();
		}
	}

#ifdef SUMDEBUG
	cout << "# Vertices: " << cg_copy->num_vertices() << " # Edges: " << cg_copy->num_edges() <<  endl;
#endif
	if (verbose){
		cout << "+++++++++++++++++" << " Summary Edge Overview " << "+++++++++++++++++" << endl;
		for(const auto & cs_id: g.callsite_map){
			auto *cs_obj = cs_id.second;
			cout << "Func_id: " << cs_obj->self_func_id << " cs_id: " << cs_obj->id << endl;
			for(const auto & se: cs_obj->input2output){
				cout << "(" << se.first << ", " << se.second << ")" << endl;
			}
		}
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
	}
}

void Tabulation::clear_summary() {
	for(const auto & item : this->g.callsite_map){
		item.second->summary_edge.clear();
	}
}

void Tabulation::forward_reach_multi_sinks(int src, vector<int> &reached_sinks) {
	clear_summary();
	vector<iEdge> self_work_list;
#ifdef IFDSDEBUG
	cout << "IFDS path: " << endl;
#endif
//	cout << "here4: func " << g[76270].func_id << " " << src << endl;
	self_work_list.emplace_back(src, src);
	reached_sinks.emplace_back(src);
	set<int> self_visited_node;
	while (!self_work_list.empty()){
		iEdge e = self_work_list.back();
		self_work_list.pop_back();
		int main = e.first;
		int sid = e.second;
		int sid_type = this->g[sid].kind;
#ifdef IFDSDEBUG
		cout << sid << "(" << sid_type  << ")" << endl;
#endif
		if(self_visited_node.count(sid)){
			continue;
		}
		self_visited_node.insert(sid);

		int curr_func_id = this->g[sid].func_id;

		if (sid_type == NodeType::INPUT){
			auto cs_idr = g.input2csid.equal_range(sid);
			for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
				auto * cs_obj = g.callsite_map[cs_id->second];
				if (!cs_obj->summary_edge.count(sid)){
					for(const auto & ce: cs_obj->call_edges){
						if(sid == ce.first) {
							build_summary(ce.second, reached_sinks);
						}
					}
				}
				auto se_r = cs_obj->summary_edge.equal_range(sid);
				for(auto se = se_r.first; se != se_r.second; se ++){
//					cout << "use se: (" << se->first << ", " << se->second << endl;
					self_work_list.emplace_back(main, se->second);
					reached_sinks.emplace_back(se->second);
				}
			}
			EdgeList succs = g.out_edges(sid);
			for(const auto & succ : succs){
				if (g[succ].kind == NodeType::ARG){
					continue;
				}
				self_work_list.emplace_back(main, succ);
				reached_sinks.emplace_back(succ);
			}
		} else {
			EdgeList succs = g.out_edges(sid);
			for(const auto & succ : succs){
				self_work_list.emplace_back(main, succ);
				reached_sinks.emplace_back(succ);
			}
		}
	}
}

void Tabulation::build_summary(int src, vector<int> &reached_sinks) {
//	cout << "here5: func " << g[76270].func_id << endl;
	int curr_func_id = this->g[src].func_id;
	auto *func_obj = this->g.function_map[curr_func_id];
	const auto &rets = func_obj->rets;

	set<int> self_visited_node;
	vector<iEdge> self_work_list;
	set<iEdge> self_summary_edge;

#ifdef IFDSDEBUG
	cout << "current arg: " << src << endl;
#endif
	self_work_list.emplace_back(src, src);
	reached_sinks.emplace_back(src);
	while(!self_work_list.empty()){
		iEdge e = self_work_list.back();
		self_work_list.pop_back();
#ifdef SUMDEBUG
		cout << "pop WL: " << "(" << e.first << ", " << e.second << ")" << endl;
#endif
		int main = e.first;
		int vid = e.second;
		if (self_visited_node.count(vid)){
			continue;
		}

		self_visited_node.insert(vid);
		int vid_type = g[vid].kind;
		int vid_func_id = g[vid].func_id;
#ifdef IFDSDEBUG
		cout << vid << "(" << vid_type  << ")" << endl;
#endif

#ifdef SUMDEBUG
		cout << "start: " << vid << " type: " << vid_type << " func_id: " << curr_func_id << endl;
#endif

		if(vid_type == NodeType::INPUT){
			auto cs_idr = g.input2csid.equal_range(vid);
			for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
#ifdef SUMDEBUG
				cout << "input-cs: " << cs_id->second << endl;
#endif
				auto * cs_obj = g.callsite_map[cs_id->second];
				if (!cs_obj->summary_edge.count(vid)){
					for(const auto & ce: cs_obj->call_edges){
						if(vid == ce.first){
#ifdef IFDSDEBUG
							cout << "enter ce, build se: (" << ce.first << ", " << ce.second  << ")" << "cs: " << cs_obj->id << " callee func: " << cs_obj->callee_func_id << endl;
#endif
							build_summary(ce.second, reached_sinks);

						}
					}
				}

				auto se_r = cs_obj->summary_edge.equal_range(vid);
				for(auto se = se_r.first; se != se_r.second; se++){
#ifdef IFDSDEBUG
					cout << "use sum-edge at cs " << cs_obj->id << "(" << se->first << ", " << se->second << ")" << endl;
#endif
//					cout << "use se at cs: " << cs_obj->id << " (" << se->first << ", " << se->second << ")" << endl;
					self_work_list.emplace_back(main, se->second);
					reached_sinks.emplace_back(se->second);
				}
			}
			EdgeList succs = g.out_edges(vid);
			for(const auto & succ : succs){
				if (g[succ].kind == NodeType::ARG){
					continue;
				}
				self_work_list.emplace_back(main, succ);
				reached_sinks.emplace_back(succ);
			}
		} else if (vid_type == NodeType::RET){
			if (vid_func_id != curr_func_id){
				cout << "ret func_id not equal: (" << vid << "." << vid_func_id << " , " << main << "." << curr_func_id << endl;
			}
			assert(vid_func_id == curr_func_id);
//			cout << "found se: (" << src << ", " << vid << ")" << endl;
			self_summary_edge.insert(iEdge(src, vid));
#ifdef IFDSDEBUG
			cout << "reach ret:" << vid << endl;
			cout << "insert arg-ret " << "(" << src << ", " << vid << ")" << " in graph. " << endl;
#endif
		}  else {
			EdgeList succs = g.out_edges(vid);
			for(const auto & succ : succs){
				if (this->g[succ].func_id != curr_func_id){
					cout << "func_id not equal: (" << vid << "." << vid_func_id << "." << curr_func_id << " , " << succ << "." << g[succ].func_id << endl;
					cout << "vid type: " << vid_type << " succ type: " << g[succ].kind << endl;
					cout << "arg: " << src << endl;
				}
				assert(this->g[succ].func_id == curr_func_id);
				self_work_list.emplace_back(main, succ);
				reached_sinks.emplace_back(succ);
#ifdef SUMDEBUG
				cout << "normal propagate: (" << main << ", " << succ << ")" << endl;
#endif
			}
		}
#ifdef SUMDEBUG
		print_path_edge();
		print_summary_edge();
#endif
	}
	// postprocessing: add input-output edge to callers
	auto cs_idr = g.arg2csid.equal_range(src);
	for(auto cs_id = cs_idr.first; cs_id != cs_idr.second; cs_id++){
		auto *cs_obj = g.callsite_map[cs_id->second];
		for(const auto & ce : cs_obj->call_edges){
			for(const auto & re : cs_obj->ret_edges){
				if (self_summary_edge.count(iEdge(ce.second, re.first))){
//					cout << "add i-o at cs :"  << cs_obj->id << " (" << ce.first << ", " << re.second << ")" << endl;
					cs_obj->summary_edge.insert(iEdge(ce.first, re.second));
#ifdef IFDSDEBUG
					cout << "At cs " << cs_obj->id << "callee func: " << cs_obj->callee_func_id << " insert in-out: " << "(" << ce.first << ", " << re.second << ")" << endl;
#endif
				}
			}
		}
	}
}


