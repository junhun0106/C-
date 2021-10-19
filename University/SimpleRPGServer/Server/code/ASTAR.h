#pragma once

#include "stdafx.h"
#include "CollisionCheck.h"

class PathNode {
public:
	int x, y; // 노드의 위치
	int g; // value_factor 시작 점에서 목표까지의 최소 비용
	int h; // 목표까지의 근사 비용
	int f; // degree 노드 n의 비용 g+h
	bool block; // 블록인지 아닌지
	PathNode *parent;
	vector<PathNode*> around_node;

public:
	PathNode() {
		x = y = 0;
		g = 0;
		h = 0;
		f = 0;
		around_node.clear();
		around_node.reserve(8);
	}
	PathNode(int a, int b) {
		x = a, y = b;
		g = h = f = 0;
		around_node.clear();
		around_node.reserve(8);
	}
	PathNode(int a, int b, bool c) {
		x = a, y = b;
		g = h = f = 0;
		block = c;
		around_node.clear();
		around_node.reserve(8);
	}
	void push_back_around_node(PathNode *node) {
		around_node.push_back(node);
	}
	int GoalbyDistance(int target_x, int target_y) {
		int dest_x = target_x - x;
		int dest_y = target_y - y;
		return (dest_x * dest_x) + (dest_y * dest_y);
	}
	void MakeAroundNode(int target_x, int target_y) {
		//around_node.clear();

		auto begin = around_node.begin();
		auto e = around_node.end();

		while (begin != e) {
			(*begin)->g = 1;
			(*begin)->h = (*begin)->GoalbyDistance(target_x, target_y);
			(*begin)->f = (*begin)->g + (*begin)->h;
			(*begin)->parent = nullptr;
			begin++;
		}

	}
	~PathNode() {

	}
};
class lesscomp {
	bool reverse;
public:
	bool operator()(const PathNode* lhs, const PathNode* rhs) {
		return (lhs->f > rhs->f);
	}
};

class re_priority_queue : public priority_queue<PathNode*, vector<PathNode*>, lesscomp> {
public:
	bool find(PathNode* val) const {
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
	re_priority_queue Open_list;
	list<PathNode*> close_list;
public:
	void Initialize_astat() {
		while (!Open_list.empty())
			Open_list.pop();
		close_list.clear();
	}
	bool SerchOpenlist(PathNode* node) {
		return Open_list.find(node);
	}
	bool SerchCloselist(PathNode *node) {
		auto f = find(close_list.begin(), close_list.end(), node);
		if (f != close_list.end()) return true;
		return false;
	}
};

class ASTAR {
	multimap<int, PathNode*> v_path;
	CCollisionCheck *space;
public:
	ASTAR();
	~ASTAR();
	void SetSpace(CCollisionCheck *s) { space = s; }
	bool NodeFind(PathNode* node, PathNode** child);
	void CreatePathNode();
	int compless_cost(PathNode *l, PathNode *r) {
		if (l->g > r->g) return l->g;
		return r->g;
	}
	pair<float, float> SerchPath(PathNode *start_node, int target_x, int target_y);
};
