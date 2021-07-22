#include "ASTAR.h"

/*****************************************/
// PATH NODE //
/*****************************************/
PATHNODE::PATHNODE() {
	x_pos = y_pos = 0;
	g = h = f = 0;
	block = PATH;
	around_child_node.clear();
	around_child_node.reserve(8);
}
PATHNODE::PATHNODE(int x, int y) {
	x_pos = x;
	y_pos = y;
	g = h = f = 0;
	block = PATH;
	around_child_node.clear();
	around_child_node.reserve(8);
}
PATHNODE::PATHNODE(int x, int y, bool b) {
	x_pos = x;
	y_pos = y;
	block = b;
	g =	h = f = 0;
	around_child_node.clear();
	around_child_node.reserve(8);
}
PATHNODE::~PATHNODE() {
	auto begin = around_child_node.begin();
	auto end = around_child_node.end();
	PATHNODE *ptr;
	while (begin != end) {
		ptr = (*begin);
		begin++;
		delete ptr;
	}
}
int PATHNODE::start_to_goal_distance(int target_x, int target_y) {
	int dist_x = target_x - x_pos;
	int dist_y = target_y - y_pos;
	return (dist_x * dist_x) + (dist_y * dist_y);
}
void PATHNODE::make_around_child_node(int target_x, int target_y) {
	auto begin = around_child_node.begin();
	auto end = around_child_node.end();
	while (begin != end) {
		(*begin)->g = 1;
		(*begin)->h = (*begin)->start_to_goal_distance(target_x, target_y);
		(*begin)->f = (*begin)->g + (*begin)->h;
		(*begin)->parent = nullptr;
		begin++;
	}
}

/*****************************************/
//	A START //
/*****************************************/
CASTAR::CASTAR()
{
	path_list.clear();
}
CASTAR::~CASTAR()
{
	path_list.clear();
}
bool CASTAR::find_node(PATHNODE* node, PATHNODE** mappingnode) {
	auto begin = path_list.find(node->GetXpos());
	auto end = path_list.end();
	while (begin != end) {
		if ((*begin).second->GetXpos() != node->GetXpos()) break;
		if ((*begin).second->GetBlock() == BLOCK) {
			begin++;
			continue;
		}
		if ((*begin).second->GetYpos() != node->GetYpos()) {
			begin++;
			continue;
		}
		else if ((*begin).second->GetYpos() == node->GetYpos())
		{
			*mappingnode = (*begin).second;
			return true;
		}
		begin++;
	}
	return false;
}
void CASTAR::PathThreadFunction(int start_key, int range) {
	//0~500
	//501~1000
	//1001~1500
	//1501~2000
	if (start_key == 1) start_key = 0;
	auto begin = path_list.find(start_key);
	auto end = path_list.lower_bound(range);
	while (begin != end) {
			PATHNODE *node = (*begin).second;
			for (auto k = 0; k < 8; k++) {
				PATHNODE findnode;
				int i = node->GetXpos();
				int j = node->GetYpos();
				if (i - 1 < 0) continue;
				if (j - 1 < 0) continue;
				if (i + 1 > 2000) continue;
				if (j + 1 > 2000) continue;
				switch (k) {
				case 0: 
					findnode = PATHNODE(i - 1, j, PATH);
					break;
				case 1: 
					findnode = PATHNODE(i + 1, j, PATH);
					break;
				case 2:
					findnode = PATHNODE(i, j - 1, PATH);
					break;
				case 3: 
					findnode = PATHNODE(i, j + 1, PATH);
					break;
				case 4: 
					findnode = PATHNODE(i - 1, j - 1, PATH); 
					break;
				case 5: 
					findnode = PATHNODE(i - 1, j + 1, PATH);
					break;
				case 6: 
					findnode = PATHNODE(i + 1, j - 1, PATH);
					break;
				case 7: 
					findnode = PATHNODE(i + 1, j + 1, PATH);
					break;
				default: cout << "Unvalid Node" << endl;
					break;
				}
				PATHNODE* child_node;
				if (find_node(&findnode, &child_node)) {
					node->around_child_node.push_back(child_node);
				}
			}
		begin++;
	}
}
void CASTAR::CreatePath(CSpacePartition* space) {
	path_list.clear();
	thread_vector.reserve(5);
	auto start = chrono::high_resolution_clock::now();
	for (auto i = 0; i < 2000; ++i) {
		for (auto j = 0; j < 2000; ++j) {
			if (true == space->Check_Collision(i, 0, j, 0.2f)) continue;
			PATHNODE *node = new PATHNODE(i, j, PATH);
			path_list.insert(make_pair(i, node));
		}
	}
	auto du = chrono::high_resolution_clock::now() - start;

	cout << "Create Time : " << chrono::duration_cast<chrono::seconds>(du).count() << " sec" << endl;

	start = chrono::high_resolution_clock::now();
	int start_key = 0;
	int range = 400;
	for (auto i = 0; i < 5; ++i) {
		thread_vector.push_back(new thread{ [&]() {PathThreadFunction(start_key + 1, range); } });
		start_key += 400;
		range += 400;
	}
	for (auto th : thread_vector) th->join();
	du = chrono::high_resolution_clock::now() - start;
	for (auto th : thread_vector) delete th;
	cout << "Find Time : " << chrono::duration_cast<chrono::seconds>(du).count() << " sec" << endl;
	/*auto begin = path_list.begin();
	auto end = path_list.end();
	while (begin != end) {
		PATHNODE *node = (*begin).second;
		static int i = 100;
		if ((*begin).second->GetXpos()  == i) {
			cout << i << endl;
			i += 100;
		}
		for (auto k = 0; k < 8; k++) {
			PATHNODE findnode;
			int i = node->GetXpos();
			int j = node->GetYpos();
			if (i - 1 < 0) continue;
			if (j - 1 < 0) continue;
			if (i + 1 > 2000) continue;
			if (j + 1 > 2000) continue;
			switch (k) {
			case 0: 
				findnode = PATHNODE(i - 1, j, PATH);
				break;
			case 1: 
				findnode = PATHNODE(i + 1, j, PATH);
				break;
			case 2:
				findnode = PATHNODE(i, j - 1, PATH);
				break;
			case 3: 
				findnode = PATHNODE(i, j + 1, PATH);
				break;
			case 4: 
				findnode = PATHNODE(i - 1, j - 1, PATH); 
				break;
			case 5: 
				findnode = PATHNODE(i - 1, j + 1, PATH);
				break;
			case 6: 
				findnode = PATHNODE(i + 1, j - 1, PATH);
				break;
			case 7: 
				findnode = PATHNODE(i + 1, j + 1, PATH);
				break;
			default: cout << "Unvalid Node" << endl;
				break;
			}
			PATHNODE* child_node;
			if (find_node(&findnode, &child_node)) {
				node->around_child_node.push_back(child_node);
			}
		}
		begin++;
	}*/
}
pair<float, float> CASTAR::SerchPath(PATHNODE* start_node, int target_x, int target_y) {
	PATHNODE* target_node = new PATHNODE(target_x, target_y);
	if (false == find_node(target_node, &target_node)) {
		cout << "Unvalid Target Position" << endl;
		pair<float, float> position(start_node->GetXpos(), start_node->GetYpos());
		delete target_node;
		return position;
	}
	if (false == find_node(start_node, &start_node)) {
		cout << "Unvalid Monster Position" << endl;
		pair<float, float> position(start_node->GetXpos() + 1, start_node->GetYpos() + 1);
		return position;
	}
	int h = start_node->start_to_goal_distance(target_x, target_y);
	int f = 0 + h;
	start_node->SetGHFP(0, h, f);
	ASTAR_LIST astar_list;
	astar_list.ASTARInitialize();
	astar_list.open_list.push(start_node);

	auto start_time = GetTickCount();
	while (!astar_list.open_list.empty()) {

		// time out 만들기

		PATHNODE* best_node = astar_list.open_list.top();
		astar_list.open_list.pop();
		if (best_node->GetXpos() == target_x &&
			best_node->GetYpos() == target_y) {
			pair<float, float> position;
			auto pos = astar_list.close_list.begin();
			pos++;
			if ((*pos)->GetXpos() == start_node->GetXpos() &&
				(*pos)->GetYpos() == start_node->GetYpos()) pos++;
			position.first = (*pos)->GetXpos();
			position.second = (*pos)->GetYpos();
			return position;
		}
		best_node->make_around_child_node(target_x, target_y);
		for (auto b = best_node->around_child_node.begin();
		b != best_node->around_child_node.end(); ++b) {
			PATHNODE *n = (*b);
			int new_g = best_node->GetG() + comp_less_cost(best_node, n);
			if ((astar_list.search_open_list(n) ||
				astar_list.search_close_list(n))
				&& n->GetG() <= new_g) continue;
			h = n->start_to_goal_distance(target_x, target_y);
			f = new_g + h;
			n->SetGHFP(new_g, h, f, best_node);
			if (astar_list.search_close_list(n))
				astar_list.close_list.remove(n);
			if (!astar_list.search_open_list(n))
				astar_list.open_list.push(n);
		}
		astar_list.close_list.push_back(best_node);
	}
	// 어디에서도 return 하지 못했다면, 갈 곳이 없다는 뜻.
	pair<float, float> position(start_node->GetXpos(), start_node->GetYpos());
	return position;	
}
