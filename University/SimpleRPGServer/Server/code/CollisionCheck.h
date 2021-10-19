#pragma once

#include "Objects.h"

class CCollisionCheck
{
	int map_size;
	int section_value;
	int one_section_size;
	int final_section_size;
	int num_of_space;

	vector<multimap<float, CObjects*>> object_manager;
public:
	CCollisionCheck();
	~CCollisionCheck();

	void BuildWorld();
	void BuildDungeon();
	void Initialize_space_division(int size, int sect_value = 3, int want_visible = 1);
	int serch_space(float x, float y);
	bool check_collision(float x, float y);
	bool find_object(int x, int y);

	vector<multimap<float, CObjects*>> get() { return object_manager; }
};

