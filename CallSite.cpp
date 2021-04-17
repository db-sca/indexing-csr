//
// Created by chaowyc on 7/3/2021.
//

#include "CallSite.h"

CallSite::CallSite() {}

CallSite::CallSite(int id, int callee_func_id, int self_func_id) {
	this->id = id;
	this->callee_func_id = callee_func_id;
	this->self_func_id = self_func_id;
}

//CallSite::~CallSite(){
//	call_edges.clear();
//	ret_edges.clear();
//	summary_edges.clear();
//}