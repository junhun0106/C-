#include "ASTAR.h"



ASTAR::ASTAR()
{
	v_path.clear();
}
ASTAR::~ASTAR()
{
	v_path.clear();
}
bool ASTAR::NodeFind(PathNode* node, PathNode** child) {
	auto b = v_path.find(node->x);
	auto e = v_path.end();

	while (b != e) {
		if ((*b).second->block == false) {
			b++;
			continue;
		}
		if ((*b).second->y == node->y) {
			*child = (*b).second;
			return true;
		}
		b++;
	}

	return false;
}
void ASTAR::CreatePathNode() {
	v_path.clear();
	for (auto i = 0; i < 400; ++i) {
		for (auto j = 0; j < 400; ++j) {
			bool block = true;
			if (space->find_object(i, j)) {
				block = false;
			}
			PathNode* node = new PathNode(i, j, block);
			v_path.insert(make_pair(i, node));
		}
	}
	auto b = v_path.begin();
	auto e = v_path.end();
	while (b != e) {
		PathNode *node = (*b).second;
		if (node->block == false)
		{
			b++;
			continue;
		}
		for (auto k = 0; k < 4; k++) {
			PathNode* find_node = nullptr;
			int i = node->x;
			int j = node->y;
			switch (k) {
			case 0: {
				if (i - 1 < 0) continue;
				find_node = new PathNode(i - 1, j, true);
				break;
			}
			case 1: {
				find_node = new PathNode(i + 1, j, true);
				break;
			}
			case 2: {
				if (j - 1 < 0) continue;
				find_node = new PathNode(i, j - 1, true);
				break;
			}
			case 3: {
				find_node = new PathNode(i, j + 1, true);
				break;
			}
			}
			PathNode* child_node;
			if (NodeFind(find_node, &child_node)) {
				node->push_back_around_node(child_node);
			}
			delete find_node;
		}
		b++;
	}
	std::cout << "bulid_up" << endl;
}
pair<float, float> ASTAR::SerchPath(PathNode *start_node, int target_x, int target_y) {
	bool first = false;
	PathNode* target_node = new PathNode(target_x, target_y, true);
	if (NodeFind(target_node, &target_node) == false) {
		delete target_node;
		std::cout << "Unvalid Target" << endl;
		pair<float, float> p;
		p.first = start_node->x;
		p.second = start_node->y;
		return p;
	}
	ASTAR_LIST astar_list;
	if (false == NodeFind(start_node, &start_node)) {
		std::cout << "유효하지 않는 공간에 있는 몬스터 입니다.";
		pair<float, float> p;
		p.first = p.first + 1;
		p.second = p.second + 1;
		return p;
	}
	start_node->g = 0;
	start_node->h = start_node->GoalbyDistance(target_x, target_y);
	start_node->f = start_node->g + start_node->h;
	start_node->parent = nullptr;
	astar_list.Open_list.push(start_node);
	while (!astar_list.Open_list.empty()) {
		PathNode *best = astar_list.Open_list.top(); // 갈 수 있는 노드 중 비용이 가장 적은 것이 top에 있다.
		astar_list.Open_list.pop(); // 오픈에 들어오면 뺀다. 
			if (best->x == target_x && best->y == target_y) {
			pair<float, float> p;
			astar_list.close_list.sort([](PathNode* lhs, PathNode* rhs) {return (lhs->f > rhs->f); });
			auto start_pos = ((astar_list.close_list.begin()));
			start_pos++;
			if ((*start_pos)->x == start_node->x &&
				(*start_pos)->y == start_node->y) start_pos++;
			p.first = (*start_pos)->x;
			p.second = (*start_pos)->y;
			return p;
		} // 현재 위치가 이미 도착점과 같은 지점.
		best->MakeAroundNode(target_x, target_y);
		// 차일드 노드들의 비용을 다시 계산.
		for (auto b = best->around_node.begin(); b != best->around_node.end(); ++b) { // for each successpr n' of n // n하고 근접한 이동 가능한 노드들
			if (astar_list.SerchCloselist((*b))) continue; // 이미 도착지점이 아니라고 판단 한 것.
			PathNode *n = (*b);
			int new_g = best->g + compless_cost(best, n);

			if ((astar_list.SerchOpenlist(n) ||  // n이 오픈 리스트에 있거나 = n은 이미 와본 곳 이다.
				astar_list.SerchCloselist(n))  // n이 클로즈 리스트에 있거나 = n은 이미 아니라고 판단 한 곳이다.
				&& n->g <= new_g) {
				continue;
			}
			n->g = new_g;
			n->h = n->GoalbyDistance(target_x, target_y);
			n->f = n->g + n->h;
			n->parent = best;
			if (astar_list.SerchCloselist(n)) {
				astar_list.close_list.remove(n);
			}// ?? 있을 수 없는 일인데...
			if (!astar_list.SerchOpenlist(n))
			{
				astar_list.Open_list.push(n);
			}
		}
		astar_list.close_list.push_back(best);
	}
	astar_list.Initialize_astat();
	pair<float, float> p;
	p.first = start_node->x;
	p.second = start_node->y;
	return p;
}
