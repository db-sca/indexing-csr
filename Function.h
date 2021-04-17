//
// Created by chaowyc on 8/3/2021.
//

#ifndef IFDS_FUNCTION_H
#define IFDS_FUNCTION_H

#include <set>
using namespace std;

class Function {
public:
	set<int> args;
	set<int> rets;
	set<pair<int, int>> arg2ret;
	int id;
public:
	Function(int);
};


#endif //IFDS_FUNCTION_H
