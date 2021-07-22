#pragma once
#include "stdafx.h"
#include "Objects.h"

class CSpacePartition
{
	int m_nMapSize;
	int m_nsection_value;
	int m_nOne_Section_size;
	int m_nFinal_Section_Number;

	int m_nSpace_number;

	vector<multimap<float, CObjects*>> m_ObjectsManager;

public:
	CSpacePartition();
	~CSpacePartition();

	void BuildWorldSpace();

	void Initialize_space_division(int map_size, int sect_value = 3, int want_visible = 1);
	int serch_space(float x, float y, float z);
	void static_build_up(int object_num, const vector<CObjects*>& object_list);
	bool Check_Collision(float x, float y, float z, float size);
	//bool Check_Collision(CPlayer* player);


	bool find_Object(float x, float y, float z);
	
	vector<multimap<float, CObjects*>> GetSpace() { return m_ObjectsManager; }
	multimap<float, CObjects*> GetObjectsIntheSpace(int index) { return m_ObjectsManager[index]; }
};

