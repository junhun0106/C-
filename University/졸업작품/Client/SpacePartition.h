#pragma once
#include "stdafx.h"
#include "Object.h"

class CSpacePartition
{
	int m_nMapSize;
	int m_nsection_value;
	int m_nOne_Section_size;
	int m_nFinal_Section_Number;

	int m_nSpace_number;

	int want_visible_sect;
	int want_visible_sect_and_around;
	vector<vector<pair<int, D3DXVECTOR3>>> m_vvObjectPosition;
	vector<vector<CGameObject*>> m_ObjectsManager;
	vector<int> m_visible_sect_vector;
public:
	CSpacePartition();
	~CSpacePartition();

	void BuildWorldSpace();
	void SetSpace(int index, CGameObject* pObject) { m_ObjectsManager[index].push_back(pObject); }

	void SetSpace(int index, D3DXVECTOR3 vec, int id) { m_vvObjectPosition[index].push_back(make_pair(id, vec)); }

	void Initialize_space_division(int map_size, int sect_value = 3, int want_visible = 1, bool want_vector = 0);
	int serch_space(float x, float y, float z);
	void static_build_up(int object_num, const vector<CGameObject*>& object_list);
	bool Check_Collision(float x, float y, float z, float size);
	//bool Check_Collision(CPlayer* player);


	vector<int> serch_visible_section(float x, float y, float z);

	bool ChekSpaceNum(int x, int z);
	bool find_Object(float x, float y, float z);

	vector<vector<CGameObject*>> GetSpace() { return m_ObjectsManager; }
	vector<CGameObject*> GetObjectsIntheSpace(int index) { return m_ObjectsManager[index]; }
	vector<pair<int, D3DXVECTOR3>> GetObjectPositionSpace(int index) { return m_vvObjectPosition[index]; }
	vector<int> GetVisibleSection() { return m_visible_sect_vector; }
};

