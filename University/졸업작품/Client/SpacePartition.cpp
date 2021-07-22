#include "stdafx.h"
#include "SpacePartition.h"

CSpacePartition::CSpacePartition()
{
	m_nMapSize = 0;
	m_nsection_value = 0;
	m_nOne_Section_size = 0;
	m_nFinal_Section_Number = 0;

	m_nSpace_number = 0;

	m_ObjectsManager.clear();
}


CSpacePartition::~CSpacePartition()
{
	m_ObjectsManager.clear();
}
void CSpacePartition::BuildWorldSpace() {

}
void CSpacePartition::Initialize_space_division(int map_size, int sect_value, int want_visible, bool want_vector) {
	m_nMapSize = map_size;
	m_nsection_value = sect_value;
	m_nOne_Section_size = pow(2, m_nsection_value);
	m_nFinal_Section_Number = map_size / m_nOne_Section_size;

	m_nSpace_number = pow(m_nOne_Section_size, 2);

	if (!want_vector)
		m_ObjectsManager.resize(m_nSpace_number);
	else
		m_vvObjectPosition.resize(m_nSpace_number);

	want_visible_sect = want_visible;
	want_visible_sect_and_around = 1 + want_visible_sect * 2;

}
int CSpacePartition::serch_space(float x, float y, float z) {
	int index_X = x / m_nFinal_Section_Number;
	int index_Y = y / m_nFinal_Section_Number;
	int index_Z = z / m_nFinal_Section_Number;

	int index = index_X + (index_Z * m_nOne_Section_size) + (index_Y * m_nOne_Section_size * m_nOne_Section_size);

	if (index < 0 || index >= m_nSpace_number) return 0;
	if (x < 0 || y < 0 || z < 0 ||
		x > m_nMapSize || y > m_nMapSize || z > m_nMapSize) return 0;

	return index;
}
bool CSpacePartition::ChekSpaceNum(int x, int z) {
	if (x <0 || z < 0 ||
		x > m_nOne_Section_size - 1 || z > m_nOne_Section_size - 1) return false;
	return true;
}
vector<int> CSpacePartition::serch_visible_section(float x, float y, float z) {
	int nX = x / m_nFinal_Section_Number;
	int nZ = z / m_nFinal_Section_Number;

	nX -= want_visible_sect;
	nZ -= want_visible_sect;

	m_visible_sect_vector.clear();
	for (auto i = 0; i < want_visible_sect_and_around; ++i) {
		for (auto j = 0; j < want_visible_sect_and_around; ++j) {
			int index_x = nX + i;
			int index_z = nZ + j;

			if (ChekSpaceNum(index_x, index_z)) {
				int index = index_x + (index_z * m_nOne_Section_size);
				m_visible_sect_vector.push_back(index);
			}
		}
	}
	return m_visible_sect_vector;

}
void CSpacePartition::static_build_up(int object_num, const vector<CGameObject*>& object_list) {

}
bool CSpacePartition::find_Object(float x, float y, float z) {
	int index = serch_space(x, y, z);

	auto begin = m_ObjectsManager[index].begin();
	auto end = m_ObjectsManager[index].end();
	while (begin != end) {

		begin++;
	}
	return false;
}
bool CSpacePartition::Check_Collision(float x, float y, float z, float size) {
	return false;
}
