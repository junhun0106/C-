
#pragma once

#include "stdafx.h"
#include "SpacePartition.h"

const auto BLOCK = true;
const auto PATH = false;

class PATHNODE {
	int x_pos, y_pos;
	int g; // g(n)
	int h; // h(n)
	int f; // f(n) = g(n) + h(n)
	bool block;
	PATHNODE *parent;

public:
	vector<PATHNODE*> around_child_node;

public:
	PATHNODE();
	PATHNODE(int x, int y);
	PATHNODE(int x, int y, bool b);
	int start_to_goal_distance(int target_x, int target_y);
	void make_around_child_node(int target_x, int target_y);
	
	void SetGHFP(int a, int b, int c, PATHNODE* node = nullptr) {
		g = a; h = b; f = c;
		parent = node;
	}
	int GetXpos() { return x_pos; }
	int GetYpos() { return y_pos; }
	int GetG() { return g; }
	int GetF() { return f; }

	bool GetBlock() { return block; }

	~PATHNODE();
};
class ASTARLESSCOMPARISON {
public:
	bool operator()(PATHNODE*lhs, PATHNODE *rhs) {
		return lhs->GetF() > rhs->GetF();
	}
};
class new_priority_queue 
	: public priority_queue<PATHNODE*, 
	vector<PATHNODE*>, ASTARLESSCOMPARISON>
{
public:
	bool find(PATHNODE *val) const {
		auto first = this->c.begin();
		auto last = this->c.end();
		while (first != last) {
			if ((*first) == val) return true;
			++first;
		}
		return false;
	}
};
class ASTAR_LIST {
public:
	new_priority_queue open_list;
	list<PATHNODE*> close_list;
public:
	ASTAR_LIST() {
		while (!open_list.empty()) {
			open_list.pop();
		}
		close_list.clear();
	}
	~ASTAR_LIST() {
		while (!open_list.empty()) {
			open_list.pop();
		}
		close_list.clear();
	}
	void ASTARInitialize() {
		while (!open_list.empty()) {
			open_list.pop();
		}
		close_list.clear();
	}
	bool search_open_list(PATHNODE* node) {
		return open_list.find(node);
	}
	bool search_close_list(PATHNODE* node) {
		auto f = find(close_list.begin(),
			close_list.end(), node);
		if (f != close_list.end()) return true;
		return false;
	}
};

class CASTAR
{
	vector<thread*> thread_vector;
	thread *th;
	multimap<int, PATHNODE*> path_list;
	// space info
public:
	CASTAR();
	~CASTAR();
	void CreatePath(CSpacePartition* space);
	bool find_node(PATHNODE *node, PATHNODE** child);
	int comp_less_cost(PATHNODE* lhs, PATHNODE*rhs) {
		if (lhs->GetG() > rhs->GetG()) return lhs->GetG();
		return rhs->GetG();
	}
	pair<float, float> SerchPath(PATHNODE* start_node, int target_x, int target_y);

	void PathThreadFunction(int start_key, int range);
};

