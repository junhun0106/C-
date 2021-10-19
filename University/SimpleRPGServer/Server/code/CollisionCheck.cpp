#include "CollisionCheck.h"



CCollisionCheck::CCollisionCheck()
{
	map_size = 0;
	section_value = 0;
	one_section_size = 0;
	final_section_size = 0;

	num_of_space = 0;

	object_manager.clear();
}
CCollisionCheck::~CCollisionCheck()
{

}
void CCollisionCheck::BuildWorld() {
	int x = 99.0f;
	for (auto i = 0; i < 300; ++i) {
		CObjects *object = new CObjects();
		object->SetPosition(x, i);
		int space_index = serch_space(x, i);
		object_manager[space_index].insert(make_pair(x, object));
	}

	x = 299.0f;
	for (auto i = 0; i < 300; ++i) {
		CObjects *object = new CObjects();
		object->SetPosition(x, i);
		int space_index = serch_space(x, i);
		object_manager[space_index].insert(make_pair(x, object));
	}

	x = 199.0f;
	for (auto i = 400; i > 100; --i) {
		CObjects *object = new CObjects();
		object->SetPosition(x, i);
		int space_index = serch_space(x, i);
		object_manager[space_index].insert(make_pair(x, object));
	}

	std::ifstream in_file("obtacle.txt");
	int x_pos, y_pos;
	x_pos = y_pos = 0;
	for (auto i = 0; i < 5000; ++i) {
		CObjects *object = new CObjects();
		in_file >> x_pos >> y_pos;
		object->SetPosition(x_pos, y_pos);
		int space_index = serch_space(x_pos, y_pos);
		object_manager[space_index].insert(make_pair(x, object));
	}
	in_file.close();
	
}
void CCollisionCheck::BuildDungeon() {

}
void CCollisionCheck::Initialize_space_division(int size, int sect_v, int want_v) {
	map_size = size;
	section_value = sect_v;
	one_section_size = pow(2, sect_v);
	final_section_size = map_size / one_section_size;

	num_of_space = pow(one_section_size, 2);
	object_manager.resize(num_of_space);
}
int CCollisionCheck::serch_space(float x, float y) {
	int index_x = x / final_section_size;
	int index_y = y / final_section_size;

	int index = index_x + (index_y * index_y);
	
	return index;
}
bool CCollisionCheck::check_collision(float x, float y) {
	x = (int)x, y = (int)y;
	int index = serch_space(x, y);
	auto begin = object_manager[index].begin();
	auto end = object_manager[index].end();

	while (begin != end) {
		if (true == (*begin).second->Collision_Check(x, y, x, y)) {
			//cout << (*begin).second->GetPosX() << " " << (*begin).second->GetPosY() << endl;
			return true;
		}
		begin++;
	}
	return false;
}
bool CCollisionCheck::find_object(int x, int y) {
	int index = serch_space(x, y);
	auto b = object_manager[index].begin();
	auto e = object_manager[index].end();

	while (b != e) {
		if ((*b).second->GetPosX() == x &&
			(*b).second->GetPosY() == y) {
			return true;
		}
		b++;
	}
	return false;
}